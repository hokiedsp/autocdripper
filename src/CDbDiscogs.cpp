/**
 * Libraries: libmusicbrainz5, libdiscid
 */

#include "CDbDiscogs.h"

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cctype> /* for isdigit */

#include "CDbDiscogsElem.h"
#include "CDbMusicBrainz.h"
#include "SCueSheet.h"
#include "credirect.h" // to redirect std::cerr stream
#include "utils.h"

using std::cout;
using std::endl;

using std::ostringstream;
using std::deque;
using std::string;
using std::runtime_error;
using std::to_string;

const std::string CDbDiscogs::base_url("https://api.discogs.com/");

// ---------------------------------------------------------------

/** Constructor.
 *
 *  @param[in] Client program name. If omitted or empty, uses "autorip"
 *  @param[in] Client program version. If omitted or empty, uses "alpha"
 */
CDbDiscogs::CDbDiscogs(const std::string &cname,const std::string &cversion)
    : CDbHttpBase(cname,cversion)
{
    Authorize_();
}

CDbDiscogs::~CDbDiscogs() {}

// ---------------------------------------------------------------

/**
 * @brief Clear all the matches from previous search
 */
void CDbDiscogs::Clear()
{
    Releases.clear();
}

/** If MayBeLinkedFromMusicBrainz() returns true, Query() performs a new
 *  query based on the MusicBrainz query results.
 *
 *  @param[in] MusicBrainz database object.
 *  @param[in] (Optional) UPC barcode
 *  @return    Number of matched records
 */
int CDbDiscogs::Query(CDbMusicBrainz &mbdb, const std::string upc)
{
    // Clear previous results
    Clear();

    int lastmaster = 0;

    // For each MB match,look for discogs link
    Releases.reserve(mbdb.NumberOfMatches());
    for (int i=0;i<mbdb.NumberOfMatches();i++)
    {
        // get the relation URL
        std::string url = mbdb.RelationUrl("discogs",i);

        cout << "[Discogs::Query] MB link: " << url << std::endl;

        if (url.size())
        {
            // If non-empty, parse the URL string
            // examples:
            //  http://www.discogs.com/release/2449413 // jobim songbook
            //  http://www.discogs.com/master/251798   // bill evans moon beams

            // Parse URL to extract the ID & its type: release vs. master
            // if parsing fails at any point, the MBDB entry is skipped
            const std::string wwwurl("http://www.discogs.com/");
            if (url.compare(0,wwwurl.size(),wwwurl)!=0) continue; // invalid link, move on to the next
            size_t pos = url.find_last_of('/',url.size());  // ID is found to the left of pos
            if (pos==(url.size()-1)) continue; // no # after the last slash -> invalid
            char *urlstr;
            json_int_t id = strtol(url.c_str()+pos+1, &urlstr, 10);
            if (id==0 || urlstr!=(url.c_str()+url.size())) continue; // invalid ID number
            bool isrelease = url.compare(pos-8, 8, "/release")==0;
            bool ismaster = url.compare(pos-7, 7, "/master")==0;
            if (!(isrelease||ismaster)) continue; // non-release related link

            cout << "[Discogs::Query] " << (isrelease?"Release ID: ":"Master ID") << id << "\n";

            if (isrelease)
            {
                // Get release data & create new entry
                PerformHttpTransfer_(base_url + "releases/" + std::to_string(id)); // received data is stored in rawdata
                CDbDiscogsElem release(rawdata);

                // if there is a master release associated with this release
                // and UPC or preferred country specified but not matched
                // query the master release
                if (((upc.size() && upc.compare(release.AlbumUPC())!=0)
                     || (upc.empty() && preferred_country.size() && preferred_country.compare(release.Country())!=0))
                    && release.FindInt("master_id",id))
                {
                    ismaster = true;
                    cout << "[Discogs::Query] Linked to a master release: " << id << "\n";
                }
                else
                {
                    // keep the record
                    Releases.emplace_back("");
                    Releases.back().Swap(release);
                }
            }

            // If URL is that of a master releases, query for the master release and pick its best matching version
            if (ismaster)
            {
                // get best matching release from the associated releases
                CDbDiscogsElem release = QueryMaster_(id, upc, lastmaster);

                if (release.data) // if release is not empty
                {
                    // keep the record
                    Releases.emplace_back("");
                    Releases.back().Swap(release);
                }
            }

            // Reached here indicates that new release has been added
            CDbDiscogsElem &elem = Releases.back();

            // if multi-disc release identify the offset
            if (elem.TotalDiscs()>1)
            {
                cout << "[CDbDiscogs::Query] Multi-disc release -> Determining the disc offset...";

                // copy the disc# from MB
                elem.disc = mbdb.DiscNumber(i);

                // if multi-disc release, starting track offset must be computed when new CDbDiscogsElem is created
                if (!elem.SetDiscOffset_(mbdb.TrackLengths(i)))
                {
                    cout << "failed! Discarding the record.\n";
                    Releases.pop_back();
                    continue;
                }
                else
                    cout << "success! Ntracks: " << elem.number_of_tracks << " Offset: " << elem.track_offset << "\n";

            }
        }
    }

    // return the number of matches
    return Releases.size();
}

/**
 * @brief Query helper function to analyze master release record
 *
 * This function queries Discogs database for a master release and picks a release from the list
 * of releases under the master release. The selected release will be inserted to the end of the
 * Releases member variable. The selection is based on preferred_country member variable and
 * UPC string argument.
 *
 * @param[in] master release record ID
 * @param[in] UPC barcode. Empty if not given.
 * @param[inout] ID of the last master record queried, will be updated if id of the url is different
 * @return Selected release record
 */
CDbDiscogsElem CDbDiscogs::QueryMaster_(const int id, const std::string upc, int &last_id)
{
    // Check the master release id
    CDbDiscogsElem release(""); // return value
    if (id!=last_id)
    {
        // update the id
        last_id = id;

        // retrieve the master data
        PerformHttpTransfer_(base_url+"/masters/"+std::to_string(id)); // received data is stored in rawdata
        CUtilJson master(rawdata);

        // get the URL link to its versions
        std::string url;
        if (CUtilJson::FindString(master.data,"versions_url",url))
        {

            cout << "[Discogs::MasterQuery] Versions are found at: " << url << "\n";

            // retrieve the versions data
            PerformHttpTransfer_(url); // received data is stored in rawdata
            CUtilJson versions(rawdata);

            // Get the number of pages
            json_int_t pages;
            json_t *pageinfo;
            if (versions.FindObject("pagination", pageinfo)
                    && CUtilJson::FindInt(pageinfo, "pages", pages))
            {

                cout << "[Discogs::MasterQuery] Versions are listed in " << pages << " pages\n";

                // go through the first page
                bool notfound = SelectFromMasterVersions_(release, versions, upc);

                // go over additional pages if available
                for (json_int_t p = 2; notfound && p<=pages; p++)
                {
                    PerformHttpTransfer_(url+"?page="+std::to_string(p)); // received data is stored in rawdata
                    CUtilJson versions(rawdata);

                    notfound = SelectFromMasterVersions_(release, versions, upc);
                }
            }
        }
    }

    return release; // for now
}

/**
 * @brief Select a release from releases listed under a master release
 *
 * The selection is based on the UPC given in the argument and preferred_country member
 * variable (which can be set via SetCountryPreference() member function). The calling function
 * (QueryMaster_()) is expected to create a new empty entry in Releases member variable for
 * this function to fill. If this function returns true, Releases.back() contains the selected
 * record data.
 *
 * @param[inout] Current release selection, updates if better match found
 * @param[in] JSON object containing versions: /masters/{master_id}/versions
 * @param[in] Optional UPC or empty string if none given
 * @return true if the selection is not firm.
 */
bool CDbDiscogs::SelectFromMasterVersions_(CDbDiscogsElem &elem, const CUtilJson &versions, const std::string &upc)
{
    bool rval = true; // returns true if selection is not firm
    json_t *list;
    json_t *version;
    std::string fmt;
    std::string country;
    std::string url;

    // get the version list
    versions.FindArray("versions", list);

    // traverse the array and look for the
    for(size_t index = 0;
        rval && index < json_array_size(list) && (version = json_array_get(list, index));
        index++)
    {
        bool country_match, upc_match;

        json_int_t id;
        CUtilJson::FindInt(version,"id",id);
        cout << "[Discogs::MasterVersionQuery] ID: " << id << "\n";

        // look for the format string and if it does not contain CD, skip
        if (!CUtilJson::FindString(version, "format", fmt)) continue;
        size_t pos=0;
        if (isdigit(fmt.front())) // ##xFMT notation
        {
            pos = fmt.find_first_of('x');
            if (pos==fmt.npos) continue;
            pos++;
        }
        if (fmt.compare(pos, 2, "CD")!=0) continue;

        // Retrieve the release data if (1) the first item, (2) UPC given (but not yet matched)
        // or (3) preferred country is given and matched to release's country
        if ((!rval || upc.size() ||
                (country_match = (preferred_country.size()
                 && CUtilJson::FindString(version,"country",country)
                 && country.compare(preferred_country)==0)))
            && CUtilJson::FindString(version,"resource_url",url))
        {
            if (!CUtilJson::FindString(version,"country",country))
                cout << "[Discogs::MasterVersionQuery] FindString country failed\n";

            // Potential match: retrieve the release data
            PerformHttpTransfer_(url); // received data is stored in rawdata
            CDbDiscogsElem release(rawdata); // parse JSON data

            // check for the query completion
            rval = !(((upc_match=(upc.size() && upc.compare(release.AlbumUPC())==0)))
                    || (upc.empty() && (country_match || preferred_country.empty())));
            /* UPC given & matched or Country given & matched or neither given*/

            // Overwrite the last Releases element with it
            if (upc_match || (upc.empty() && !elem.data))
            {
                cout << "[Discogs::MasterVersionQuery] Release data swapped." << endl;
                elem.Swap(release);
            }

            if (country_match) cout << "[Discogs::MasterVersionQuery] Country matched" << endl;
            else cout << "[Discogs::MasterVersionQuery] " << preferred_country << " vs. " << country << endl;

            //CUtilJson::PrintJSON(version);
        }
    }
    return rval;
}

//-------------------------------------------------------------------------------------------------

/** Returns the number of matched records returned from the last Query() call.
 *
 *  @return    Number of matched records
 */
int CDbDiscogs::NumberOfMatches() const
{
    return Releases.size();
}

/** Return a unique release ID string
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return id string if Query was successful.
 */
std::string CDbDiscogs::ReleaseId(const int recnum) const
{
    // set record
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid Release record ID."));

    return Releases[recnum].ReleaseId();
}

/** Get album title
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Title string (empty if title not available)
 */
std::string CDbDiscogs::AlbumTitle(const int recnum) const
{
    // set record
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid Release record ID."));

    return Releases[recnum].AlbumTitle();
}

/** Get album artist
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Artist string (empty if artist not available)
 */
std::string CDbDiscogs::AlbumArtist(const int recnum) const
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
std::string CDbDiscogs::AlbumComposer(const int recnum) const
{
    return "";
}

/** Get genre
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Empty string (Discogs does not support genre)
 */
std::string CDbDiscogs::Genre(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].Genre();
}

/** Get release date
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Date string (empty if genre not available)
 */
std::string CDbDiscogs::Date(const int recnum) const
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
std::string CDbDiscogs::Country(const int recnum) const
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
int CDbDiscogs::DiscNumber(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    // if somehow reaches here, data is likely corrupted or severely incomplete
    return Releases[recnum].DiscNumber();
}

/**
 * @brief Get number of discs in the release
 * @param Release record ID (0-based index). If omitted, the first record (0)
 *        is returned.
 * @return Number of discs in the release
 */
int CDbDiscogs::TotalDiscs(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    // if somehow reaches here, data is likely corrupted or severely incomplete
    return Releases[recnum].TotalDiscs();
}

/** Get label name
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Label string (empty if label not available)
 */
std::string CDbDiscogs::AlbumLabel(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].AlbumLabel();
}

/** Get album UPC (barcode)
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    UPC string (empty if title not available)
 */
std::string CDbDiscogs::AlbumUPC(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].AlbumUPC();
}

/**
 * @brief Returns number of tracks on the CD
 * @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 * @return Number of tracks
 */
int CDbDiscogs::NumberOfTracks(const int recnum) const
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
std::string CDbDiscogs::TrackTitle(int tracknum, const int recnum) const
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
std::string CDbDiscogs::TrackArtist(int tracknum, const int recnum) const
{
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
std::string CDbDiscogs::TrackComposer(int tracknum, const int recnum) const
{
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
std::string CDbDiscogs::TrackISRC(int tracknum, const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].TrackISRC(tracknum);
}

void CDbDiscogs::Authorize_()
{
    //    pid = fork();
    //    if (pid == 0) {
    //        execl("/usr/bin/xdg-open", "xdg-open", the_file, (char *)0);
    //        exit(1);
    //    }
}
