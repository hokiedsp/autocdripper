/**
 * Libraries: libmusicbrainz5, libdiscid
 */

#include "CDbMusicBrainz.h"

#include <climits>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "SCueSheet.h"

#include "CUtilXmlTree.h"
#include "CDbMusicBrainzElem.h"
#include "CDbMusicBrainzElemCAA.h"

#include "CDbAmazonElem.h"
#include "CDbAmazon.h"

#include "utils.h"

using std::cout;
using std::endl;

using std::vector;
using std::string;
using std::runtime_error;
using std::to_string;

const std::string CDbMusicBrainz::base_url = "http://musicbrainz.org/ws/2/";

/** Constructor->l785
 *
 *  @param[in] Client program name. If omitted or empty, uses "autorip"
 *  @param[in] Client program version. If omitted or empty, uses "alpha"
 */
CDbMusicBrainz::CDbMusicBrainz(const std::string &cname,const std::string &cversion)
    : CUtilUrl(cname,cversion), amazon(nullptr), CoverArtSize(2)
{}

CDbMusicBrainz::~CDbMusicBrainz()
{
    // Clear disc info collection
    Clear();

    // delete amazon database object if created
    if (amazon) delete(amazon);
}

void CDbMusicBrainz::SetGrabCoverArtFromAmazon(const bool ena)
{
    if (amazon && !ena) // disabled
    {
        delete(amazon);
        amazon = nullptr;
    }
    else if (!amazon && ena) // enabled
    {
        amazon = new CDbAmazon(dynamic_cast<const CUtilUrl&>(*this));
    }
}


/** If AllowQueryCD() returns true, Query() performs a new query for the CD info
 *  in the specified drive with its *  tracks specified in the supplied cuesheet
 *  and its length. Previous query outcome discarded. After disc and its tracks
 *  are initialized, CDDB disc ID is computed. If the computation fails, function
 *  throws an runtime_error->l785l785
 *
 *  @param[in] CD-ROM device path
 *  @param[in] Cuesheet with its basic data populated
 *  @param[in] Length of the CD in sectors
 *  @param[in] (Optional) UPC barcode
 *  @return    Number of matched records
 */
int CDbMusicBrainz::Query(const std::string &dev, const SCueSheet &cuesheet, const size_t len, const std::string cdrom_upc)
{
    // Clear the discs
    Clear();

    cout << "[CDbMusicBrainz::Query] 1. Querying with disc TOC...";

    // must build disc based on cuesheet (throws error if fails to compute discid)
    CUtilXmlTree discdata = GetNewDiscData_(cuesheet);

    const xmlNode *release_node;
    std::string url;
    if (discdata.FindArray("release-list",release_node))
    {
        cout << "[CDbMusicBrainz::Query] 2. Getting more information of each release... " << endl;

        bool upc_match = false;
        std::string id, barcode;

        // Create new MBQueries element & populate
        for (; !upc_match && release_node; release_node = release_node->next)
        {
            cout << "[CDbMusicBrainz::Query] Processing Release " << Releases.size() << "... " << endl;

            // retrieve the release ID (if failed, skip the release)
            if (!discdata.FindElementAttribute(release_node,"id",id)) continue;

            cout << "[CDbMusicBrainz::Query]    ID: " << id  << endl;

            // match disc
            int disc = DiscID_(release_node, cuesheet.Tracks.size(), cuesheet.TotalTime);

            cout << "[CDbMusicBrainz::Query]    Disc#: " << disc  << endl;

            // Check the UPC
            if (cdrom_upc.size() && discdata.FindString(release_node,"barcode",barcode))
            {
                if (barcode.size())
                {
                    // remove non-digits from barcode string before comparison
                    cleanup_upc(barcode);
                    upc_match = (cdrom_upc.compare(barcode)==0);

                    // If UPC matched, discard all previous entries
                    if (upc_match) Releases.clear();
                }
            }

            // Build release lookup URL
            url.clear();
            url = base_url + "release/" + id + "?inc=labels+artists+recordings+artist-credits+release-groups+url-rels";

            cout << "[CDbMusicBrainz::Query]    URL: " << url << endl;

            // Get release data & create new entry
            PerformHttpTransfer_(url); // received data is stored in rawdata

            // Parse the downloaded XML data
            Releases.emplace_back(rawdata,disc);

            Releases.back().PrintXmlTree();
        }

        cout << "[CDbMusicBrainz::Query] Searching for cover arts]" << endl;

        // Populate CoverArts vector
        CoverArts.reserve(Releases.size());
        for (vector<CDbMusicBrainzElem>::iterator it = Releases.begin();
             it != Releases.end();
             it ++)
        {
            // Build coverart lookup URL
            std::string url = "http://coverartarchive.org//release/" + it->ReleaseId();

            // Get release data & create new entry
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            PerformHttpTransfer_(url); // received data is stored in rawdata
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);

            // Parse the downloaded JSON data
            try
            {
                CoverArts.emplace_back(rawdata);    // if rawdata invalid, would not affect the container
            }
            catch(...)
            {}
        }
    }

    cout << "[CDbMusicBrainz::Query] Complete. Found " << Releases.size() << endl;

    // return the number of matches
    return Releases.size();
}

/** Initialize a new disc and fill it with disc info
 *  from the supplied cuesheet and length. Previously created disc
 *  data are discarded. After disc and its tracks are initialized,
 *  CDDB disc ID is computed. If the computation fails, function
 *  throws an runtime_error.
 *
 *  @param[in] Cuesheet with its basic data populated
 *  @param[in] Length of the CD in sectors
 */
CUtilXmlTree CDbMusicBrainz::GetNewDiscData_(const SCueSheet &cuesheet)
{
    // Build discid lookup URL
    std::ostringstream url;
    url << base_url << "discid/-?toc=1+" << cuesheet.Tracks.size() << "+" << cuesheet.TotalTime+150;

    // add duration of each track
    SCueTrackDeque::const_iterator itTrack;
    for (itTrack=cuesheet.Tracks.begin(); itTrack!=cuesheet.Tracks.end(); itTrack++)
    {
        const SCueTrack &track = *itTrack;
        SCueTrackIndexDeque::const_iterator itIndex;
        for (itIndex=track.Indexes.begin(); itIndex!=track.Indexes.end() && (*itIndex).number<1; itIndex++);

        if ((*itIndex).number>1) throw (std::runtime_error("Invalid cuesheet: A track is missing Index 1."));

        url << "+" << (*itIndex).time+150;
    }

    // Get release data & create new entry
    PerformHttpTransfer_(url.str()); // received data is stored in rawdata

    // Parse the downloaded XML data
    CUtilXmlTree discdata(rawdata);

    return discdata;
}

/**
 * @brief Check medium-list in a discid release to identify the disc if multi-disc set
 * @param[in] pointer to an XML node for a release
 * @param[in] number of tracks
 * @param[in] total time of the CD in sectors
 * @return disc number
 */
int CDbMusicBrainz::DiscID_(const xmlNode *release_node, const int trackcount, const size_t totaltime)
{
    int discno=1;
    int curr_disc;
    int count;
    const xmlNode *medium_node;

    if (!CUtilXmlTree::FindArray(release_node, "medium-list", medium_node, &count))
        throw(std::runtime_error("Multi-CD Release Data is missing medium/position element."));

    if (count>1)
    {
        const xmlNode *node;
        int metric = INT_MAX;

        while (medium_node && metric>0) // for each medium
        {
            if (!CUtilXmlTree::FindInt(medium_node, "position", curr_disc))
                throw(std::runtime_error("Multi-CD Release Data is missing medium/position element."));

            CUtilXmlTree::FindArray(medium_node, "track-list", node, &count);
            if (count==trackcount)
            {
                if (!CUtilXmlTree::FindArray(medium_node, "disc-list",node))
                    throw(std::runtime_error("Multi-CD Release Data is missing medium/disc-list element."));

                while (node && metric>0) // for each disc
                {
                    if (!CUtilXmlTree::FindInt(node, "sectors", count))
                        throw(std::runtime_error("Multi-CD Release Data is missing medium/disc/sectors element."));

                    count -= (int)totaltime;
                    if (count) count = abs(count);
                    if (count<metric)
                    {
                        metric = count;
                        discno = curr_disc;
                    }

                    node = node->next;
                }

            }

            medium_node = medium_node->next;
        }
    }

    return discno;
}

/** Clear all the disc entries
 */
void CDbMusicBrainz::Clear()
{
    // clear the list of matched releases and their coverart info
    Releases.clear();
}

/** Return the CDDB discid
 *
 *  @return CDDB discid if Query() has been completed successfully. Otherwise
 *          zero.
 */
std::string CDbMusicBrainz::GetDiscId() const
{
    return "";
}

/** Returns the number of matches (records) returned from the last Query() call.
 *
 *  @return    Number of matches
 */
int CDbMusicBrainz::NumberOfMatches() const
{
    return Releases.size();
};

/////////////////////////////////////////////////////////////////////////////////

/** Return a unique release ID string
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return id string if Query was successful.
 */
std::string CDbMusicBrainz::ReleaseId(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].ReleaseId();
}

/** Get album title
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Title string (empty if title not available)
 */
std::string CDbMusicBrainz::AlbumTitle(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].AlbumTitle();
}

/** Get album artist
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Artist string (empty if artist not available)
 */
std::string CDbMusicBrainz::AlbumArtist(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].AlbumArtist();
}

/** Get album composer
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Composer/songwriter string (empty if artist not available)
 */
std::string CDbMusicBrainz::AlbumComposer(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].AlbumComposer();
}

/** Get release date
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Date string (empty if genre not available)
 */
std::string CDbMusicBrainz::Date(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].Date();
}

/** Get release country
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Countery string (empty if genre not available)
 */
std::string CDbMusicBrainz::Country(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].Country();
}

/**
 * @brief Get disc number
 * @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *            is returned.
 * @return    Disc number or -1 if unknown
 */
int CDbMusicBrainz::DiscNumber(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].DiscNumber();
}

/**
 * @brief Get total number of discs in the release
 * @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *            is returned.
 * @return    Number of discs or -1 if unknown
 */
int CDbMusicBrainz::TotalDiscs(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].TotalDiscs();
}

/** Get label name
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Label string (empty if label not available)
 */
std::string CDbMusicBrainz::AlbumLabel(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].AlbumLabel();
}

/** Get catalog number
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Catalog Number string (empty if not available)
 */
std::string CDbMusicBrainz::AlbumCatNo(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].AlbumCatNo();
}

/** Get album UPC
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    UPC string (empty if title not available)
 */
std::string CDbMusicBrainz::AlbumUPC(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].AlbumUPC();
}

/** Get Amazon Standard Identification Number
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    ASIN string (empty if ASIN not available)
 */
std::string CDbMusicBrainz::AlbumASIN(const int recnum) const
{
    {
        // set disc
        if (recnum<0 || recnum>=(int)Releases.size()) // all discs
            throw(runtime_error("Invalid CD record ID."));

        return Releases[recnum].AlbumASIN();
    }
}

/** Get number of tracks
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    number of tracks
 *  @throw     runtime_error if CD record id is invalid
 */
int CDbMusicBrainz::NumberOfTracks(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].NumberOfTracks();
}

/** Get track title
 *
 *  @param[in] Track number (1-99)
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Title string (empty if title not available)
 *  @throw     runtime_error if track number is invalid
 */
std::string CDbMusicBrainz::TrackTitle(int tracknum, const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].TrackTitle(tracknum);
}

/** Get track artist
 *
 *  @param[in] Track number (1-99)
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Artist string (empty if artist not available)
 *  @throw     runtime_error if track number is invalid
 */
std::string CDbMusicBrainz::TrackArtist(int tracknum, const int recnum) const
{
    string rval;

    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].TrackArtist(tracknum);
}

/** Get track composer
 *
 *  @param[in] Track number (1-99)
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Composer string (empty if artist not available)
 *  @throw     runtime_error if track number is invalid
 */
std::string CDbMusicBrainz::TrackComposer(int tracknum, const int recnum) const
{
    string rval;

    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].TrackComposer(tracknum);
}

/** Get track ISRC
 *
 *  @param[in] Track number (1-99)
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    ISRC string
 *  @throw     runtime_error if track number is invalid
 */
std::string CDbMusicBrainz::TrackISRC(int tracknum, const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].TrackISRC(tracknum);
}

//----------------------------------------------------------------------------------------

/** Specify the preferred coverart image width
 *
 *  @param[in] Preferred width of the image
 */
void CDbMusicBrainz::SetPreferredWidth(const size_t &width)
{
    if (width>500) CoverArtSize = 0;
    else if (width>250) CoverArtSize = 1;
    else CoverArtSize = 2;
}

/** Specify the preferred coverart image height
 *
 *  @param[in] Preferred height of the image
 */
void CDbMusicBrainz::SetPreferredHeight(const size_t &height)
{
    if (height>500) CoverArtSize = 0;
    else if (height>250) CoverArtSize = 1;
    else CoverArtSize = 2;
}

//----------------------------------------------------------------------------------------

/** Check if the query returned a front cover
 *
 *  @param[in]  record index (default=0)
 *  @return     true if front cover is found.
 */
bool CDbMusicBrainz::Front(const int recnum) const
{
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    bool rval = Releases[recnum].Front();

    // if coverart not available on CAA, check if Amazon link is given
    if (!rval && amazon)
    {
        amazon->Query(Releases[recnum].AlbumASIN());
        if (amazon->NumberOfMatches()>0) rval = amazon->Front();
    }

    return rval;
}

/** Check if the query returned a back cover
 *
 *  @param[in]  record index (default=0)
 *  @return     true if back cover is found.
 */
bool CDbMusicBrainz::Back(const int recnum) const
{
    if (recnum<0||recnum>(int)CoverArts.size())
        throw(runtime_error("Invalid Record Index requested."));

    return Releases[recnum].Back();
}

/** Retrieve the front cover data.
 *
 *  @param[out] image data buffer.
 *  @param[in]  record index (default=0)
 */
UByteVector CDbMusicBrainz::FrontData(const int recnum)
{
    return DataToMemory(FrontURL(recnum));
}

/** Check if the query returned a front cover
 *
 *  @param[out] image data buffer.
 *  @param[in]  record index (default=0)
 */
UByteVector CDbMusicBrainz::BackData(const int recnum)
{
    return DataToMemory(BackURL(recnum));
}

/** Get the URL of the front cover image
 *
 *  @param[in]  Record index (default=0)
 *  @return     URL string
 */
std::string CDbMusicBrainz::FrontURL(const int recnum) const
{
    string rval;

    // check recnum validity
    if (recnum<0||recnum>(int)CoverArts.size())
        throw(runtime_error("Invalid Record Index requested."));

    // get release
    std::string id = Releases[recnum].ReleaseId();

    // loop CoverArts to look for the data
    for (std::vector<CDbMusicBrainzElemCAA>::const_iterator it = CoverArts.begin();
         rval.empty() && it!=CoverArts.end();
         it++)
    {
        if (id.compare((*it).ReleaseId())==0) rval = (*it).FrontURL(CoverArtSize);
    }

    // if coverart not available on CAA, check if Amazon link is given
    if (rval.empty() && amazon)
    {
        amazon->Query(Releases[recnum].AlbumASIN());
        if (amazon->NumberOfMatches()>0) rval = amazon->FrontURL();
    }

    return rval;
}

/** Get the URL of the back cover image
 *
 *  @param[in]  Record index (default=0)
 *  @return     URL string
 */
std::string CDbMusicBrainz::BackURL(const int recnum) const
{
    string rval;

    // check recnum validity
    if (recnum<0||recnum>(int)CoverArts.size())
        throw(runtime_error("Invalid Record Index requested."));

    // get release
    std::string id = Releases[recnum].ReleaseId();

    // loop CoverArts to look for the data
    for (std::vector<CDbMusicBrainzElemCAA>::const_iterator it = CoverArts.begin();
         rval.empty() && it!=CoverArts.end();
         it++)
    {
        if (id.compare((*it).ReleaseId())==0) rval = (*it).BackURL(CoverArtSize);
    }

    return rval;
}

/**
 * @brief Get a related URL.
 * @param[in]  Relationship type (e.g., "discogs", "amazon asin")
 * @param[in]  Record index (default=0)
 * @return URL string or empty if requestd URL type not in the URL
 */
std::string CDbMusicBrainz::RelationUrl(const std::string &type, const int recnum)
{
    string rval;

    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    // First check the RelationListList in the Release
    rval = Releases[recnum].RelationUrl(type);

    if (rval.empty()) // if none found in Release, check ReleaseGroup
    {
        string id = Releases[recnum].ReleaseGroupId();
        string url;

        // Build release lookup URL
        url.clear();
        url = base_url + "release-group/" + id + "?inc=url-rels";

        // Get release data & create new entry
        PerformHttpTransfer_(url); // received data is stored in rawdata

        // Parse the downloaded XML data
        CUtilXmlTree rgroupdata(rawdata);

        // look through all relation
        string rval;
        bool relation_notfound=true, target_notfound = true;
        const xmlNode *list, *relation;
        string target;

        if (rgroupdata.FindElement("relation-list", list))
        {
            do
            {
                if (rgroupdata.FindElementAttribute(list,"target-type",target) && target.compare("url")==0)
                {
                    relation_notfound = false;
                    for (relation = list->children;
                         relation && (target_notfound = rgroupdata.CompareElementAttribute(relation,"type",type)!=0);
                         relation = relation->next);

                    if (!target_notfound) rgroupdata.FindString(relation,"target",rval);
                }
            } while (relation_notfound && rgroupdata.FindNextElement(list,"relation-list", list));
        }
    }

    return rval;
}

/** Get track length
 *
 *  @param[in] Track number (1-99)
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Track length in seconds
 *  @throw     runtime_error if track number is invalid
 */
std::vector<int> CDbMusicBrainz::TrackLengths(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].TrackLengths();
}
