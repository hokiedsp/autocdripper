/**
 * Libraries: libmusicbrainz5, libdiscid
 */

#include "CDbDiscogs.h"

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "CDbDiscogsElem.h"
#include "CDbElemJsonBase.h"
#include "CDbMusicBrainz.h"
#include "SCueSheet.h"
#include "credirect.h" // to redirect std::cerr stream

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

    int lastmaster=0;

    // For each MB match,look for discogs link
    Releases.reserve(mbdb.NumberOfMatches());
    for (int i=0;i<mbdb.NumberOfMatches();i++)
    {
        // get the disc#
        int discno = mbdb.DiscNumber(i);

        // get the relation URL
        std::string url = mbdb.RelationUrl("discogs",i);
        if (url.size())
        {
            // If non-empty, parse the URL string
            // examples:
            //  http://www.discogs.com/release/2449413 // jobim songbook
            //  http://www.discogs.com/master/251798   // bill evans moon beams

            // convert the given URL with base_url for API query URL
            const std::string mb_base_url("http://www.discogs.com/");
            url.replace(0,mb_base_url.length(),base_url);

            // If a link to master release is given, find its oldest CD release
            if (url.compare(base_url.length(),6,"master")==0)
            {
                int id = QueryMaster_(url, lastmaster);
                if (url.empty()) continue; // duplicate master, go to next record
                lastmaster = id; // update
            }

            // Get release data
            PerformHttpTransfer_(url); // received data is stored in rawdata
            Releases.emplace_back(rawdata,discno);

            // check for the uniqueness

        }
    }

    // return the number of matches
    return Releases.size();
}

/**
 * @brief Return the oldest CD release w/in a master release
 * @param[inout] discogs API url to a master release then returns a URL to a release
 * @return true if failed to retrieve URL to a release
 * @throw runtime_error if URL does not resolve to a valid master record
 */
int CDbDiscogs::QueryMaster_(std::string &url, const int last_id)
{
    int id=0;

    PerformHttpTransfer_(url); // received data is stored in rawdata
//    CDbElemJsonBase master(rawdata);
//    master.PrintJSON();

//    FindInt_(master,"id",id);
//    if (id==last_id)
//    {
        url.clear();
//    }
//    else
//    {
//        url.clear(); // for now
//        // sort through data
//    }

    return id; // for now
}

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

/** Retrieve the disc info from specified database record
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    SDbrBase Pointer to newly created database record object. Caller is
 *             responsible for deleting the object.
 */
//SDbrBase* CDbDiscogs::Retrieve(const int recnum) const
//{
//    json_t *tracks;
//    int num_tracks;
//    string str;
//    json_int_t val;

//    // instantiate new DBR object
//    SDbrDiscogs * rec = new SDbrDiscogs;

//    // set disc
//    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
//        throw(runtime_error("Invalid Release record ID."));

//    // Grab the specified release info
//    const json_t *r = Releases[recnum];

//    rec->Title = Title_(r);
//    rec->Performer = Artist_(r);

//    rec->Rems.emplace_back("DBSRC Discogs");
//    if (FindInt_(r,"id",val))
//        rec->Rems.emplace_back("ID "+to_string(val));	// comments on the disc

//    str = Genre_(r);
//    if (str.size()) rec->Rems.emplace_back("GENRE "+str);

//    str = Date_(r);
//    if (str.size())
//        rec->Rems.emplace_back("DATE "+str);

//    str = Country_(r);
//    if (str.size())
//        rec->Rems.emplace_back("COUNTRY "+str);

//    str = Identifier_(r,"Barcode");
//    if (str.size())
//        rec->Rems.emplace_back("UPC "+str);

//    str = Identifier_(r,"ASIN");
//    if (str.size())
//        rec->Rems.emplace_back("ASIN "+str);

//    str = Label_(r);
//    if (str.size())
//        rec->Rems.emplace_back("LABEL "+str);

//    // get disc# and total # of cds if multiple-cd set
//    val = NumberOfDiscs_(r);
//    if (val>1)
//    {
//        rec->Rems.emplace_back("DISC "+to_string(discnos[recnum]));
//        rec->Rems.emplace_back("DISCS "+to_string(val));
//    }

//    // initialize tracks
//    tracks = TrackList_(r, discnos[recnum]);
//    num_tracks = json_array_size(tracks);
//    rec->AddTracks(num_tracks); // adds Tracks 1 to num_tracks

//    for (int i=0;i<num_tracks;i++)
//    {
//        json_t *track = json_array_get(tracks,i);

//        // get the track object
//        SCueTrack &rectrack = rec->Tracks[i];

//        rectrack.Title = Title_(track);
//        rectrack.Performer = Artist_(track);

//        //CISRCList *isrcs = recording->ISRCList();
//        //if (isrcs && isrcs->NumItems()>0)
//        //    rectrack.ISRC = isrcs->Item(0)->ID();
//    }

//    return rec;
//}
