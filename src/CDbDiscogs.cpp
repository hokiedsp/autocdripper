/**
 * Libraries: libmusicbrainz5, libdiscid
 */

#include "CDbDiscogs.h"

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <iomanip>

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

/** Constructor.
 *
 *  @param[in] Client program name. If omitted or empty, uses "autorip"
 *  @param[in] Client program version. If omitted or empty, uses "alpha"
 */
CDbDiscogs::CDbDiscogs(const std::string &cname,const std::string &cversion)
    : CDbHttpBase(cname,cversion), CDbJsonBase()
{
    Authorize_();
}

/** Constructor.
 *
 *  @param[in] Client program name. If omitted or empty, uses "autorip"
 *  @param[in] Client program version. If omitted or empty, uses "alpha"
 */
CDbDiscogs::CDbDiscogs(const CDbMusicBrainz &mb, const std::string &cname,const std::string &cversion)
    : CDbHttpBase(cname,cversion), CDbJsonBase()
{
    // for each discogs release URL

}

CDbDiscogs::~CDbDiscogs() {}

/** Perform a new Discogs query given a list of its release IDs
 *
 *  @param[in] collection of release IDs
 *  @return    Number of valid records
 */
int CDbDiscogs::Query(const std::deque<std::string> &list)
{
    // if empty deque given, do nothing
    if (list.empty()) return 0;

    ClearReleases_();

    // look up all releases
    deque<string>::const_iterator it;
    for (it=list.begin();it!=list.end();it++)
    {
        PerformHttpTransfer_(base_url+"releases/"+*it);
        AppendRelease_(data);
        discnos.push_back(1);
    }

    // return the number of matches
    return Releases.size();
}

/** Perform a new Discogs query given a list of its release IDs
 *
 *  @param[in] collection of release IDs
 *  @return    Number of valid records
 */
int CDbDiscogs::Query(const std::string &id, const int disc)
{
    ClearReleases_();

    // look up all releases
    PerformHttpTransfer_(base_url+"releases/"+id);
    AppendRelease_(data);

    discnos.push_back(disc);

    // return the number of matches
    return Releases.size();
}

/** If IsSearchable() returns true, Search() performs a new album search based on
 *  album title and artist. If search is not supported or did not return any match,
 *  Search() returns zero.
 *
 *  @param[in] Album title
 *  @param[in] Album artist
 *  @param[in] If true, immediately calls Read() to populate disc records.
 *  @param[in] Network time out in seconds. If omitted or negative, previous value
 *             will be reused. System default is 10.
 *  @return    Number of matched records
 */
int CDbDiscogs::Search(const std::string &title, const std::string &artist, const bool autofill, const int timeout)
{
    return 0;
}

/** Returns the CD record ID associated with the specified genre. If no matching
 *  record is found, it returns -1.
 *
 *  @return Matching CD record ID.
 */
int CDbDiscogs::MatchByGenre(const std::string &genre) const
{
    return -1;
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

    return Title_(Releases[recnum]);
}

std::string CDbDiscogs::Title_(const json_t* json)
{
    string titlestr;
    if (FindString_(json,"title",titlestr)) return titlestr;
    return "";
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

    return Artist_(Releases[recnum]);
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

    return Genre_(Releases[recnum]);
}

std::string CDbDiscogs::Genre_(const json_t* release)
{
    json_t *genres;
    if (!FindArray_(release,"genres",genres)) return "";
    return json_string_value(json_array_get(genres,0));
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

    return json_array_size(TrackList (Releases[recnum], discnos[recnum]));
}


std::string CDbDiscogs::Artist_(const json_t* json)
{
    string astr, joinstr, str;
    bool more = true; // if true, more artist names are expected
    size_t index;
    json_t *artists, *value;

    // Look for artist entries
    if (FindArray_(json,"artists",artists))
    {
        json_array_foreach(artists, index, value)
        {
            // look for the name string
            if (!FindString_(value,"name",str) || str.empty()) break;
            astr += str;

            // look for the join string and append it to the artist string
            more = FindString_(value,"join", joinstr);
            if (more && joinstr.size())
            {
                if (joinstr.substr(0,1).find_first_of(" ,;:./\\])>|")==string::npos) astr += ' ';
                astr += joinstr;
                if (joinstr.substr(joinstr.size()-1,1).find_first_of(" /\\[(<|")==string::npos) astr += ' ';
            }
            else
            {
                more = false;
                break;
            }
        }
    }

    // If none found or more expected, also check extraartists entries
    if (more && FindArray_(json,"extraartists",artists))
    {
        json_array_foreach(artists, index, value)
        {
            // look for the name string
            if (!FindString_(value,"name",str) || str.empty()) break;
            astr += str;

            // look for the join string and append it to the artist string
            if (!FindString_(value,"join", joinstr) || joinstr.empty()) break;
            if (joinstr.substr(0,1).find_first_of(" ,;:./\\])>|")==string::npos) astr += ' ';
            astr += joinstr;
            if (joinstr.substr(joinstr.size()-1,1).find_first_of(" /\\[(<|")==string::npos) astr += ' ';
        }
    }

    return astr;
}

std::string CDbDiscogs::Label_(const json_t* release)
{
    json_t *labels;
    string labelstr;

    if (!FindArray_(release,"labels",labels)) return "";
    labels = json_array_get(labels,0); // get the first label entry
    if (FindString_(labels,"name",labelstr)) return labelstr;
    return "";
}

std::string CDbDiscogs::Date_(const json_t* release)
{
    json_int_t yr;
    string str;
    if (FindString_(release,"released",str)) return str;
    else if (FindInt_(release,"year",yr)) return to_string(yr);
    else return"";
}

std::string CDbDiscogs::Country_(const json_t* release)
{
    string str;
    if (FindString_(release,"country",str)) return str;
    else return "";
}

std::string CDbDiscogs::Identifier_(const json_t* release, const std::string type)
{
    size_t index;
    json_t *idarray, *id;
    string str;

    if (!FindArray_(release,"identifiers",idarray)) return "";

    // Look for the entry with requested type
    json_array_foreach(idarray, index, id)
    {
        // if Identifier type does not match, go to next
        if (CompareString_(id, "type", type)) continue;

        if (FindString_(id,"value",str)) return str;
    }

    // if reaches here, Sought Identifier is not present in the record
    return "";
}

/**
 * @brief Get number of discs in the release
 * @param Release record ID (0-based index). If omitted, the first record (0)
 *        is returned.
 * @return Number of discs in the release
 */
int CDbDiscogs::NumberOfDiscs(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return NumberOfDiscs_(Releases[recnum]);
}

int CDbDiscogs::NumberOfDiscs_(const json_t *release)
{
    json_t *array, *elem;
    size_t index;
    string str;

    // look for format_quantity
    json_int_t nodiscs;
    if (FindInt_(release,"format_quantity",nodiscs))
        return (int)nodiscs;

    // look for qty entry under formats
    if (FindArray_(release,"formats",array))
    {

        // Look for format entries
        json_array_foreach(array, index, elem)
        {
            if (CompareString_(elem,"name","CD") && FindString_(elem,"qty",str))
            {
                try
                {
                    return std::stoi(str); // if qty given as a number, return it
                }
                catch(...)
                {}
            }
        }
    }

    // if for some reason, formats/qty is not found, check for existence of sub track field
    {
        if (FindArray_(release,"tracklist",array))
        {
            // If all tracks contain sub_tracks, assume multi-CD release, else single-cd release
            json_array_foreach(array, index, elem)
            {
                if (!FindArray_(elem,"sub_tracks",elem)) return 1;
            }

            // If reached here, all tracks contains sub_tracks -> multi-cd release
            return json_array_size(array); // return # of tracks as the # of cds
        }
    }

    // if somehow reaches here, data is likely corrupted or severely incomplete
    return 0;
}

json_t* CDbDiscogs::TrackList (const json_t *release, const int discno)
{
    json_t *tracks;

    if (!FindArray_(release,"tracklist",tracks))
        throw(runtime_error("Tracklist not found."));

    // if multi-disc set
    if (NumberOfDiscs_(release)>1)
    {
        FindArray_(json_array_get(tracks,discno-1),"sub_tracks",tracks);
    }

    return tracks;
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

    return Label_(Releases[recnum]);
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

    return Identifier_(Releases[recnum],"Barcode");
}

/** Retrieve the disc info from specified database record
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    SDbrBase Pointer to newly created database record object. Caller is
 *             responsible for deleting the object.
 */
SDbrBase* CDbDiscogs::Retrieve(const int recnum) const
{
    json_t *tracks;
    int num_tracks;
    string str;
    json_int_t val;

    // instantiate new DBR object
    SDbrDiscogs * rec = new SDbrDiscogs;

    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid Release record ID."));

    // Grab the specified release info
    const json_t *r = Releases[recnum];

    rec->Title = Title_(r);
    rec->Performer = Artist_(r);

    rec->Rems.emplace_back("DBSRC Discogs");
    if (FindInt_(r,"id",val))
        rec->Rems.emplace_back("ID "+to_string(val));	// comments on the disc

    str = Genre_(r);
    if (str.size()) rec->Rems.emplace_back("GENRE "+str);

    str = Date_(r);
    if (str.size())
        rec->Rems.emplace_back("DATE "+str);

    str = Country_(r);
    if (str.size())
        rec->Rems.emplace_back("COUNTRY "+str);

    str = Identifier_(r,"Barcode");
    if (str.size())
        rec->Rems.emplace_back("UPC "+str);

    str = Identifier_(r,"ASIN");
    if (str.size())
        rec->Rems.emplace_back("ASIN "+str);

    str = Label_(r);
    if (str.size())
        rec->Rems.emplace_back("LABEL "+str);

    // get disc# and total # of cds if multiple-cd set
    val = NumberOfDiscs_(r);
    if (val>1)
    {
        rec->Rems.emplace_back("DISC "+to_string(discnos[recnum]));
        rec->Rems.emplace_back("DISCS "+to_string(val));
    }

    // initialize tracks
    tracks = TrackList (r, discnos[recnum]);
    num_tracks = json_array_size(tracks);
    rec->AddTracks(num_tracks); // adds Tracks 1 to num_tracks

    for (int i=0;i<num_tracks;i++)
    {
        json_t *track = json_array_get(tracks,i);

        // get the track object
        SCueTrack &rectrack = rec->Tracks[i];

        rectrack.Title = Title_(track);
        rectrack.Performer = Artist_(track);

        //CISRCList *isrcs = recording->ISRCList();
        //if (isrcs && isrcs->NumItems()>0)
        //    rectrack.ISRC = isrcs->Item(0)->ID();
    }

    return rec;
}


void CDbDiscogs::Authorize_()
{
    //    pid = fork();
    //    if (pid == 0) {
    //        execl("/usr/bin/xdg-open", "xdg-open", the_file, (char *)0);
    //        exit(1);
    //    }
}
