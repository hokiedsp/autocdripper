#include "CDbDiscogsElem.h"

#include <cstdlib>
//#include <ctime>
//#include <sstream>
//#include <iomanip>
//#include <numeric> // for std::accumulate

#include "utils.h"

CDbDiscogsElem::CDbDiscogsElem(const std::string &data, const int d, const int offset)
    : CUtilJson(data), disc(d), track_offset(offset)
{
    SetDiscSize_();
}

CDbDiscogsElem::CDbDiscogsElem(const std::string &data, const int d, const std::vector<int> &tracklengths)
    : CUtilJson(data), disc(d)
{
    SetDiscOffset_(tracklengths);
}

/**
 * @brief Exchanges the content with another CDbDiscogsElem object
 * @param Another CDbDiscogsElem object
 */
void CDbDiscogsElem::Swap(CDbDiscogsElem &other)
{
    json_t *tempdata = data;
    data = other.data;
    other.data = tempdata;

    int tempdisc = disc;
    disc = other.disc;
    other.disc = tempdisc;
}

/**
 * @brief Fill number_of_tracks
 */
void CDbDiscogsElem::SetDiscSize_()
{
    // gather track info
    number_of_tracks = 0;
    TraverseTracks_([&] (const json_t *t, const json_t *p)
    {
        number_of_tracks++;
        return true;
    } );
}

/**
 * @brief Determine track offset for multi-disc release
 *
 * This function fills track_offset & number_of_tracks member variables.
 *
 * @param[in] vector of lengths of CD tracks in seconds
 */
bool CDbDiscogsElem::SetDiscOffset_(const std::vector<int> &cdtracks)
{
    // gather track info
    std::vector<int> alltracks;
    alltracks.reserve(128);
    TraverseTracks_([&] (const json_t *t, const json_t *p)
    {
        // lambda function to be executed by TraverseTracks_
        std::string duration;
//        struct std::tm tm;
        if (FindString(t,"duration",duration))
        {
            const char *str;
            const char *str0 = duration.c_str();
            int T = strtol(str0, const_cast<char**>(&str), 10);
            if (str==str0) return false; // integer character not found
            while (str && *str==':')
            {
                str0 = ++str;
                T *= 60;
                T += strtol(str0, const_cast<char**>(&str), 10);
                if (str==str0) return false; // integer character not found
            }
            if (str) return false; // extrenous characters at the end

            alltracks.push_back(T);

            // NOT implemented in GCC4.8, must wait till GCC5.0
            //            // convert duration string to
            //            std::istringstream ss(duration);
            //            ss >> std::get_time(&tm, "%H:%M:%S");

            //            if (ss.fail())
            //            {
            //                ss.clear();
            //                ss.seekg(0,ss.beg);
            //                ss >> std::get_time(&tm, "%M:%S");

            //                if (ss.fail())
            //                {
            //                    ss.clear();
            //                    ss.seekg(0,ss.beg);
            //                    ss >> std::get_time(&tm, "%S"); // not Discogs legitimate, but just in case
            //                }
            //            }

            //            // if unknown duration format, invalid release
            //            if (ss.fail()) return false; // hopefully it would never get here



            // convert to seconds and store
            //alltracks.push_back(tm.tm_sec+60*tm.tm_min+3600*tm.tm_hour);
        }
        return true;
    } );


    number_of_tracks = cdtracks.size();
    track_offset = 0;
    int Nlags = alltracks.size()-number_of_tracks;
    if (Nlags>0) // exact match, no duration check
    {
        typedef std::vector<int>::const_iterator cintvectiter;
        typedef std::vector<int>::iterator intvectiter;
        auto compute_align_metric = [](intvectiter first1, intvectiter last1, cintvectiter first2, cintvectiter last2)
        {
            int metric = abs((*first1++)-(*first2++));
            while (first1!=last1) metric += abs((*first1++)-(*first2++));
            return metric;
        };

        cintvectiter cd_begin = cdtracks.begin();
        cintvectiter cd_end = cdtracks.end();
        intvectiter it = alltracks.begin();
        int min = 100;

        for (int lag = 0; lag<=Nlags && min>0; lag++)
        {
            int metric = compute_align_metric(it,(it++)+number_of_tracks,cd_begin,cd_end);
            if (metric < min)
            {
                min = metric;
                track_offset = lag;
            }
        }
    }

    return Nlags>=0;
}

/**
 * @brief Traverses tracklist array and calls Callback() for every track-type element
 * @param[in] Callback function taking a pointer to track (or subtrack) JSON element
 *            and its parent track (only for subtrack, NULL if element is track) and
 *            returns true to go to next track, false to quit traversing
 */
void CDbDiscogsElem::TraverseTracks_(std::function<bool (const json_t *track, const json_t *parent)> Callback) const
{
    // start the traversal of tracklist
    json_t *tracks, *track;
    bool gotonext = true;

    // get the tracklist array
    if (!FindArray("tracklist", tracks)) return;

    // for each track of the tracklist
    for(size_t index = 0;
        gotonext && index < json_array_size(tracks) && (track = json_array_get(tracks, index));
        index++)
    {
        json_t *subtracks, *subtrack;
        std::string type;

        if (FindString(track,"type",type))
        {
            if (type.compare("track")==0) // found a track
            {
                // call the callback function
                gotonext = Callback(track,NULL);
            }
            else if (type.compare("index")==0 && FindArray(track,"sub_tracks",subtracks)) // contain sub_tracks
            {
                for(size_t subindex = 0;
                    gotonext && subindex < json_array_size(tracks) && (subtrack = json_array_get(subtracks, subindex));
                    subindex++)
                {
                    // call the callback function
                    gotonext = Callback(subtrack,track);
                }
            }
        }
    }
}

/**
 * @brief Find JSON object for the specified track
 * @param[inout] track number between 1 and 99 -> 0
 * @return pointer to a track JSON object
 */
const json_t* CDbDiscogsElem::FindTrack_(const size_t tracknum, const json_t **index) const
{
    if (!tracknum)
        throw(std::runtime_error("Invalid tracknum, must be positive."));

    const json_t *track;  // return value
    size_t num = tracknum;
    size_t offset = track_offset;

    // if calling function requesting the pointer to index track, initialize to NULL
    if (index) *index = NULL;

    // traverse the tracklist looking for the requested track
    TraverseTracks_([&] (const json_t *t, const json_t *p)
    {
        if (offset) offset--;
        else num--;

        if (!num) // found the track
        {
            track = t;
            if (index) *index = p;
        }

        return num;
    } );

    if (num || !track)
        throw(std::runtime_error("Track not found."));

    return track;
}

// -----------------------------------------------------------------------------------------

/** Return a unique release ID string
 *
 *  @return id string if Query was successful.
 */
std::string CDbDiscogsElem::ReleaseId() const
{
    std::string rval;
    json_int_t rvalint;

    if (FindInt(data,"id",rvalint))
        rval = std::to_string(rvalint);

    return rval;
}

/** Get album title
 *
 *  @return    Title string (empty if title not available)
 */
std::string CDbDiscogsElem::AlbumTitle() const
{
    return Title_(data);
}

/** Get album artist
 *
 *  @return    Artist string (empty if artist not available)
 */
std::string CDbDiscogsElem::AlbumArtist() const
{
    return Artist_(data);
}

/** Get album composer
 *
 *  @return    Composer/songwriter string (empty if artist not available)
 */
std::string CDbDiscogsElem::AlbumComposer() const
{
    std::string rval;
    return rval;
}

/** Get genre
 *
 *  @return    Genre string (empty if genre not available)
 */
std::string CDbDiscogsElem::Genre() const
{
    json_t *genres;
    if (!FindArray(data,"genres",genres)) return "";
    return json_string_value(json_array_get(genres,0));

}

/** Get release date
 *
 *  @return    Date string (empty if genre not available)
 */
std::string CDbDiscogsElem::Date() const
{
    json_int_t yr;
    std::string str;

    if (!FindString(data,"released",str)
            && FindInt(data,"year",yr))
        str = std::to_string(yr);

    return str;
}

/** Get release country
 *
 *  @return    Countery string (empty if genre not available)
 */
std::string CDbDiscogsElem::Country() const
{
    std::string str;
    FindString(data,"country",str);
    return str;
}

/**
 * @brief Get disc number
 * @return    Disc number or -1 if unknown
 */
int CDbDiscogsElem::DiscNumber() const
{
    return disc;
}

/**
 * @brief Get total number of discs in the release
 * @return    Number of discs or -1 if unknown
 */
int CDbDiscogsElem::TotalDiscs() const
{
    int rval = 1;
    json_t *formats;

    try
    {
        std::string qty;
        if (FindArray("formats",formats)
                && FindString(json_array_get(formats,0),"qty",qty))
            rval = std::stoi(qty);
    }
    catch (...) {} // if exception thrown, ignore and return 1

    return rval;
}

/** Get label name
 *
 *  @return    Label string (empty if label not available)
 */
std::string CDbDiscogsElem::AlbumLabel() const
{
    json_t *labels;
    std::string labelstr;

    if (FindArray("labels",labels))
    {
        labels = json_array_get(labels,0); // get the first label entry
        FindString(labels,"name",labelstr);
    }

    return labelstr;
}

/** Get album UPC
 *
 *  @return    UPC string (empty if UPC not available)
 */
std::string CDbDiscogsElem::AlbumUPC() const
{
    std::string upc = Identifier("Barcode");
    if (upc.size()) cleanup_upc(upc);
    return upc;
}

/** Get number of tracks
 *
 *  @return    number of tracks
 *  @throw     runtime_error if CD record id is invalid
 */
int CDbDiscogsElem::NumberOfTracks() const
{
    return number_of_tracks;
}

/** Get track title
 *
 *  @param[in] Track number (1-99)
 *  @return    Title string (empty if title not available)
 *  @throw     runtime_error if track number is invalid
 */
std::string CDbDiscogsElem::TrackTitle(int tracknum) const
{
    std::string rval;

    // initialize tracks
    const json_t *parent;
    const json_t *track = FindTrack_(tracknum, &parent);

    if (parent!=NULL && Genre().compare("Classical")==0)
    {
        rval = Title_(parent) + '-';
    }

    rval += Title_(track);

    return rval;
}

/** Get track artist
 *
 *  @param[in] Track number (1-99)
 *  @return    Artist string (empty if artist not available)
 *  @throw     runtime_error if track number is invalid
 */
std::string CDbDiscogsElem::TrackArtist(int tracknum) const
{
    std::string rval;

    // initialize tracks
    const json_t *parent;
    const json_t *track = FindTrack_(tracknum, &parent);

    rval = Artist_(track);

    return rval;
}

/** Get track composer
 *
 *  @param[in] Track number (1-99)
 *  @return    Composer string (empty if artist not available)
 *  @throw     runtime_error if track number is invalid
 */
std::string CDbDiscogsElem::TrackComposer(int tracknum) const
{
    return "";
}

/** Get track ISRC
 *
 *  @param[in] Track number (1-99)
 *  @return    ISRC string
 *  @throw     runtime_error if track number is invalid
 */
std::string CDbDiscogsElem::TrackISRC(int tracknum) const
{
    return "";
}

std::string CDbDiscogsElem::Identifier(const std::string type) const
{
    size_t index;
    json_t *idarray, *id;
    std::string str;

    if (!FindArray("identifiers",idarray)) return "";

    // Look for the entry with requested type
    json_array_foreach(idarray, index, id)
    {
        // if Identifier type does not match, go to next
        if (CompareString(id, "type", type)) continue;

        if (FindString(id,"value",str)) return str;
    }

    // if reaches here, Sought Identifier is not present in the record
    return "";
}

std::string CDbDiscogsElem::Title_(const json_t* data) // maybe release or track json_t
{
    std::string titlestr;
    FindString(data, "title", titlestr);
    return titlestr;
}

std::string CDbDiscogsElem::Artist_(const json_t* data, const int artisttype) // maybe release or track json_t
{
    std::string astr, joinstr, str;
    bool more = true; // if true, more artist names are expected
    size_t index;
    json_t *artists, *value;

    // Look for artist entries
    if (FindArray(data, "artists",artists))
    {
        json_array_foreach(artists, index, value)
        {
            // look for the name string
            if (!FindString(value,"name",str) || str.empty()) break;
            astr += str;

            // look for the join string and append it to the artist string
            more = FindString(value,"join", joinstr);
            if (more && joinstr.size())
            {
                if (joinstr.substr(0,1).find_first_of(" ,;:./\\])>|")==std::string::npos) astr += ' ';
                astr += joinstr;
                if (joinstr.substr(joinstr.size()-1,1).find_first_of(" /\\[(<|")==std::string::npos) astr += ' ';
            }
            else
            {
                more = false;
                break;
            }
        }
    }

    // If none found or more expected, also check extraartists entries
    if (more && FindArray(data, "extraartists",artists))
    {
        json_array_foreach(artists, index, value)
        {
            // look for the name string
            if (!FindString(value,"name",str) || str.empty()) break;
            astr += str;

            // look for the join string and append it to the artist string
            if (!FindString(value,"join", joinstr) || joinstr.empty()) break;
            if (joinstr.substr(0,1).find_first_of(" ,;:./\\])>|")==std::string::npos) astr += ' ';
            astr += joinstr;
            if (joinstr.substr(joinstr.size()-1,1).find_first_of(" /\\[(<|")==std::string::npos) astr += ' ';
        }
    }

    return astr;
}
