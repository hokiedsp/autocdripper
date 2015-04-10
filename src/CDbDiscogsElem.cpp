#include "CDbDiscogsElem.h"

#include <stdexcept>
#include <cstdlib>
#include <climits>
//#include <ctime>
//#include <sstream>
//#include <iomanip>
//#include <numeric> // for std::accumulate

#include <iostream>

#include "utils.h"

CDbDiscogsElem::CDbDiscogsElem(const std::string &data, const int d, const int offset)
    : CUtilJson(data), disc(d), track_offset(offset)
{
    SetDiscSize_();
}

CDbDiscogsElem::CDbDiscogsElem(const std::string &data, const int d, const std::vector<int> &tracklengths)
    : CUtilJson(data), disc(d)
{
    if (!SetDiscOffset_(tracklengths))
        throw(std::runtime_error("Release without valid track information."));
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
    TraverseTracks_([&] (const json_t *t, const json_t *p, size_t pidx, const json_t *h, size_t hidx)
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
    TraverseTracks_([&] (const json_t *t, const json_t *p, size_t pidx, const json_t *h, size_t hidx)
    {
        // lambda function to be executed by TraverseTracks_
        std::string duration;

        if (FindString(t,"duration",duration))
        {
            char *str;
            const char *str0 = duration.c_str();
            int T = strtol(str0, &str, 10);
            while (str!=str0 && str && *str==':')
            {
                str0 = ++str;
                T *= 60;
                T += strtol(str0, &str, 10);
            }
            if (*str) return false; // extrenous characters at the end

            // add the duration to the vector
            alltracks.push_back(T);
        }
        return true;
    } );

    // compare discogs track info to CD's
    number_of_tracks = cdtracks.size();
    track_offset = 0;
    int Nlags = alltracks.size()-number_of_tracks;
    if (Nlags>0) // more tracks in release than on CD
    {
        // determine the track offset
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
        int min = INT_MAX;
        for (int lag = 0; lag<=Nlags && min>0; lag++)
        {
            int metric = compute_align_metric(it, it+number_of_tracks, cd_begin, cd_end);
            if (metric < min)
            {
                min = metric;
                track_offset = lag;
            }
            it++;
        }

        // determine the disc #
        if (track_offset==0) // -> undisputably the first disc
        {
            disc = 1;
        }
        else
        {
            // Expects Pos entry on the first track is expected to begin with a disc #.
            // Any preceding non-digits are skipped and first number is taken as the disc #.
            // If no number of found or the first # is less than 2, disc# is probably not
            // displayed in the field. If so, disc variable remain unchanged.
            std::string position;
            const json_t* track = FindTrack_(1);
            CUtilJson::FindString(track,"position",position);
            const char* str = position.c_str();
            while (*str && !isdigit(*str)) str++;
            if (*str)
            {
                int T = strtol(str, NULL, 10);
                if (T>1) disc = T;
            }
        }
    }

    return Nlags>=0; // if # of tracks in the discogs release is shorter than CD's, invalid release
}

/**
 * @brief Traverses tracklist array and calls Callback() for every track-type element
 * @param[in] Callback function
 *
 * Callback (lambda) function:
 * - Called on every JSON object under tracklist" with its "type_"=="track"
 * - Input arguments:
 *   const json_t *track: Pointer to the current JSON "track" object
 *   const json_t *parent: Pointer to the parent JSON "index track" object if track
 *                         is a sub_track. Or NULL if track is not a sub_track
 *   const int &pidx: sub_track index if parent is not NULL. Otherwise, unknown
 *   const json_t *heading: Pointer to the last seen JSON "heading track" object.
 *                          NULL if there has been none.
 *   const int &hidx: track index (only counting the main tracks) w.r.t. heading.
 *                    Unknonwn if heading is NULL,
 */
void CDbDiscogsElem::TraverseTracks_(
            std::function<bool (const json_t *track,
                                const json_t *parent, size_t pidx,
                                const json_t *heading, size_t hidx)> Callback) const
{
    // start the traversal of tracklist
    json_t *tracks, *track, *heading=NULL;
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
        size_t subindex, headindex;

        if (FindString(track,"type_",type))
        {
            if (type.compare("track")==0) // found a track
            {
                // call the callback function
                gotonext = Callback(track,NULL,subindex,heading,index-headindex);
            }
            else if (type.compare("index")==0 && FindArray(track,"sub_tracks",subtracks)) // contain sub_tracks
            {
                // call the callback function on each subtrack
                for(subindex = 0;
                    gotonext && subindex < json_array_size(subtracks) && (subtrack = json_array_get(subtracks, subindex));
                    subindex++)
                    gotonext = Callback(subtrack,track,subindex,heading,index-headindex);
            }
            else if (type.compare("heading")==0)
            {
                heading = track;
                headindex = index;
            }
        }
    }
}

/**
 * @brief Find JSON object for the specified track
 * @param[inout] track number between 1 and 99 -> 0
 * @return pointer to a track JSON object
 */
const json_t* CDbDiscogsElem::FindTrack_(const size_t tracknum, const json_t **index, size_t *suboffset, const json_t **heading, size_t *headoffset) const
{
    if (!tracknum)
        throw(std::runtime_error("Invalid tracknum, must be positive."));

    const json_t *track;  // return value
    size_t num = tracknum;
    size_t offset = track_offset;

    // if calling function requesting the pointer to index track, initialize to NULL
    if (index) *index = NULL;

    // traverse the tracklist looking for the requested track
    TraverseTracks_([&] (const json_t *t, const json_t *p, size_t pidx, const json_t *h, size_t hidx)
    {
        if (offset) offset--;
        else num--;

        if (!num) // found the track
        {
            track = t;
            if (index) *index = p;
            if (suboffset) *suboffset = pidx;
            if (heading) *heading = h;
            if (headoffset) *headoffset = hidx;
        }

        return num;
    } );

    if (num || !track)
        throw(std::runtime_error("Track not found."));

    return track;
}

/**
 * @brief return the total number of sub_tracks listed under an index track
 * @param[in] pointer to the index track JSON object
 * @return number of sub_tracks or 0 if failed
 */
size_t CDbDiscogsElem::NumberOfSubTracks_(const json_t *track)
{
    json_t *subtracks;
    size_t rval = 0;

    if (FindArray(track,"sub_tracks",subtracks))
        rval = json_array_size(subtracks);

    return rval;
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

/** Get catalog number
 *
 *  @return    CatNo string (empty if not available)
 */
std::string CDbDiscogsElem::AlbumCatNo() const
{
    json_t *labels;
    std::string catno;

    if (FindArray("labels",labels))
    {
        labels = json_array_get(labels,0); // get the first label entry
        FindString(labels,"catno",catno);
    }

    return catno;
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
    size_t pidx;
    bool showtitle=true;
    const json_t *track = FindTrack_(tracknum, &parent, &pidx, NULL, NULL);

    // For a work with multiple movements, Discogs uses sub_tracks for its
    // movement tracks (not always followed though...)
    if (parent!=NULL)
    {
        showtitle = NumberOfSubTracks_(parent)>1;
        rval = Title_(parent);
        if (showtitle) rval += ": " + itoroman(pidx+1) + ". ";
    }

    // add the main title of the track
    if (showtitle) rval += Title_(track);

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
    if (rval.empty() && parent) rval = Artist_(parent);

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
    std::string rval;
    json_t *idarray;
    if (FindArray("identifiers",idarray))
    {

        // Look for the entry with requested type
        json_t *id;
        for(size_t index = 0;
            rval.empty() && index < json_array_size(idarray) && (id = json_array_get(idarray, index));
            index++)
        {
            // if Identifier type does not match, go to next
            if (CompareString(id, "type", type)) continue;

            if (FindString(id,"value",str)) return str;
        }
    }

    return rval;
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
