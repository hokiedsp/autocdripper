#include "CDbDiscogsElem.h"

#include <stdexcept>
#include <cstdlib>
#include <climits>
#include <cctype>
//#include <ctime>
//#include <sstream>
//#include <iomanip>
//#include <numeric> // for std::accumulate

#include <iostream>

#include "utils.h"

CDbDiscogsElem::CDbDiscogsElem(const std::string &rawdata, const int d, const int offset)
    : CUtilJson(rawdata), disc(d), track_offset(offset)
{
    SetDiscSize_();
    AnalyzeArtists_();
}

CDbDiscogsElem::CDbDiscogsElem(const std::string &rawdata, const int d, const std::vector<int> &tracklengths)
    : CUtilJson(rawdata), disc(d)
{
    if (!SetDiscOffset_(tracklengths))
        throw(std::runtime_error("Release without valid track information."));

    AnalyzeArtists_();
}

/**
 * @brief Exchanges the content with another CDbDiscogsElem object
 * @param Another CDbDiscogsElem object
 */
void CDbDiscogsElem::Swap(CDbDiscogsElem &other)
{
    CUtilJson::Swap(other);
    std::swap(disc,other.disc);
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
SCueArtists CDbDiscogsElem::AlbumArtist() const
{
    return Performer_(data);
}

/** Get album composer
 *
 *  @return    Composer/songwriter string (empty if artist not available)
 */
SCueArtists CDbDiscogsElem::AlbumComposer() const
{
    return Composer_(data);
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

/** @brief Get track title
 *
 *  Track title is intelligently obtained.
 *  Case 1. main track - track title
 *  Case 2. sub_track - parent index track title + ": " + [index#] + ". " sub_track title
 *     Exception 1. If only 1 sub_track is given under an index track, index# and sub_track
 *     title are omitted.
 *     Exception 2. If sub_track index# is found in its title (either as an Arabic or Roman number)
 *     [index#] is omitted. Note that inclusion of movement # is discouraged by Discogs.
 *  Case Unaccounted: Some submitters use heading track to provide the main title of the
 *     work and subsequent toplevel tracks for its movements. This function ignores all the heading
 *     tracks, thus only movement names will be added to the cuesheet. Discogs entry must be fixed
 *     to fix this case.
 *
 *  @param[in] Track number (1-99)
 *  @return    Title string (empty if title not available)
 *  @throw     runtime_error if track number is invalid
 */
std::string CDbDiscogsElem::TrackTitle(int tracknum) const
{
    std::string rval;
    std::string movt_num;

    // initialize tracks
    const json_t *parent;
    size_t pidx;
    bool showtitle=true;
    const json_t *track = FindTrack_(tracknum, &parent, &pidx);

    // For a work with multiple movements, Discogs uses sub_tracks for its
    // movement tracks (not always followed though...)
    if (parent!=NULL)
    {
        rval = Title_(parent);
        rval += ": ";

        // only show the movement titles if there are more than 1 movement
        showtitle = NumberOfSubTracks_(parent)>1;
        if (showtitle) movt_num = itoroman(++pidx);
    }

    if (showtitle)
    {
        // add the main title of the (sub)track
        std::string track_title = Title_(track);

        // add movement # only if it's not already included in the track title
        if (movt_num.size())
        {
            bool excluded = track_title.compare(0,movt_num.size(),movt_num)!=0;
            if (excluded)
            {
                std::string movt_arabic = std::to_string(pidx);
                if (track_title.compare(0,movt_arabic.size(),movt_arabic)!=0)
                    rval += movt_num + ". ";
            }
        }

        // append the track title
        rval += track_title;
    }

    return rval;
}

/** Get track artist
 *
 *  @param[in] Track number (1-99)
 *  @return    Artist string (empty if artist not available)
 *  @throw     runtime_error if track number is invalid
 */
SCueArtists CDbDiscogsElem::TrackArtist(int tracknum) const
{
    std::string rval;

    // initialize tracks
    const json_t *parent;
    const json_t *track = FindTrack_(tracknum, &parent);

    return Performer_(track);
}

/** Get track composer
 *
 *  @param[in] Track number (1-99)
 *  @return    Composer string (empty if artist not available)
 *  @throw     runtime_error if track number is invalid
 */
SCueArtists CDbDiscogsElem::TrackComposer(int tracknum) const
{
    // initialize tracks
    const json_t *parent;
    const json_t *track = FindTrack_(tracknum, &parent);

    return Composer_(track);
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
            if (CompareString(id, "type", type)==0)
                FindString(id,"value",rval);
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

SCueArtists CDbDiscogsElem::Performer_(const json_t* data) // maybe release or track json_t
{
    SCueArtists rval;
    std::string joinstr, str;
    json_t *artists;

    // Get credits to check for composer
    json_t *credits;
    if (!FindArray(data, "extraartists",credits)) credits = NULL;

    // Look for artist entries
    if (FindArray(data, "artists", artists))
    {
        json_t *artist;
        size_t num_artists = json_array_size(artists);
        for (size_t i = 0; i<num_artists && (artist = json_array_get(artists,i)); i++)
        {
            // if non-composer and given a valid name
            if (!IsComposer_(artist, credits)
                    && ((FindString(artist,"anv",str) && str.size()) // get album specific alternate name first
                        || (FindString(artist,"name",str) && str.size()))) // if not given, use the Discogs' name
            {
                if (joinstr.size()) rval.back().joiner = joinstr;

                rval.emplace_back();
                rval.back().name = str;

                // look for the next join string and append it to the artist string
                FindString(artist, "join", joinstr);
            }
        }
    }

    return rval;
}

SCueArtists CDbDiscogsElem::Composer_(const json_t* data) const// maybe release or track json_t
{
    SCueArtists rval;
    std::string joinstr, str;
    json_t *artists;
    json_t *artist;

    // Get credits to check for composer
    json_t *credits;
    if (!FindArray(data, "extraartists",credits)) credits = NULL;

    // Look for artist entries
    if (FindArray(data, "artists", artists))
    {
        size_t num_artists = json_array_size(artists);
        for (size_t i = 0; i<num_artists && (artist = json_array_get(artists,i)); i++)
        {
            // if non-composer and given a valid name
            if (IsComposer_(artist, credits)
                    && ((FindString(artist,"anv",str) && str.size()) // get album specific alternate name first
                        || (FindString(artist,"name",str) && str.size()))) // if not given, use the Discogs' name
            {
                if (joinstr.size()) rval.back().joiner = joinstr;

                rval.emplace_back();
                rval.back().name = str;

                // look for the next join string and append it to the artist string
                FindString(artist,"join", joinstr);
            }
        }
    }

    // if composer not given in main artists list, look in the credits
    if (rval.empty())
    {
        std::cout << "Composer not found in artists, checking extraartists\n";

        bool notfound = true;
        size_t num_artists = json_array_size(credits);
        for (size_t i = 0; notfound && i<num_artists && (artist = json_array_get(credits,i)); i++)
        {
            PrintJSON(artist);

            // if composer and given a valid name
            if (IsComposer_(artist, credits, {"Composed By"}) // only look for "Composed By"
                    && ((FindString(artist,"anv",str) && str.size()) // get album specific alternate name first
                        || (FindString(artist,"name",str) && str.size()))) // if not given, use the Discogs' name
            {
                notfound = false;
                rval.emplace_back();
                rval.back().name = str;
                // if album credits, make sure not depending on position
            }
        }
    }

    // if track composer still not found, look in the album credits
    if (rval.empty() && credits!=album_credits)
    {
        // TODO
    }

    return rval;
}

bool CDbDiscogsElem::IsComposer_(const json_t* artist, const json_t* extraartists, const std::vector<std::string> &keywords)
{
    bool rval = false;

    // get artist's role (check in the given artist JSON object first, then in the extraartists JSON array)
    std::string role;
    FindString(artist,"role",role);
    if (role.empty())
    {
        artist = FindArtist_(ArtistId_(artist),extraartists);
        if (artist) FindString(artist,"role",role);
    }

    // if role is found, look for must creator
    if (role.size())
    {
        std::vector<std::string>::const_iterator it;
        for (it=keywords.begin(); !rval && it!=keywords.end(); it++)
        {
            size_t pos = role.find((*it));
            if (pos!=role.npos) // if keyword is found
            {
                // make sure it is a standalone word
                if (pos==0 || isspace(role[pos-1]))
                {
                    std::cout << "   verified a space before " << *it << "\n";

                    pos += (*it).size();
                    rval = pos >= role.size() || isspace(role[pos]);
                }
            }
        }
    }

    return rval;
}

/**
 * @brief Look for the artist with the ID in the array of artists
 * @param id
 * @param artists
 * @return JSON object corresponds to the artist under search or NULL if not found
 */
json_t *CDbDiscogsElem::FindArtist_(const json_int_t id, const json_t* artists)
{
    json_t *rval = NULL;
    json_t *artist;
    for (size_t i=0;
         !rval && i<json_array_size(artists) && (artist = json_array_get(artists, i));
         i++)
    {
        if (id==ArtistId_(artist)) rval = artist;
    }
    return rval;
}

/**
 * @brief Get artist's Discogs ID
 * @param artist JSON object (an element of artists or extraartists arrays)
 * @return integer ID
 */
json_int_t CDbDiscogsElem::ArtistId_(const json_t* artist)
{
    json_int_t rval = -1;
    FindInt(artist,"id",rval);
    return rval;
}

/**
 * @brief Analyze Artists lists to fill various_performers, various_composers, and album_credits
 */
void CDbDiscogsElem::AnalyzeArtists_()
{
    various_performers = false;    // false if album has unique set of performing artists
    various_composers = false;     // false if album has a single composer
    album_credits = NULL;      // fixed link to the release's extraartists

    // if element has no data, nothing to do
    if (!data) return;

    // Get and store the extraartist JSON array for later use
    FindArray(data, "extraartists", album_credits);

    // Look for artist entries
    json_t *artists;
    if (FindArray(data, "artists", artists) && json_array_size(artists)>0)
    {
        std::string name;
        json_t *artist = json_array_get(artists,0);
        FindString(artist,"name",name);

        // Look for name == "Various"
        various_performers = name.compare("Various")==0;

        if (various_performers) // If various artist, possibly various_composers as well
        {
            various_composers = true;
        }
        else // specific artists given, check if they are composers (or any kind of music creaters)
        {
            // check if more than one composers are listed
            int num_artists = json_array_size(artists);
            int num_composers = IsComposer_(artist, album_credits);
            for (int i=1;
                 i<num_artists && (artist = json_array_get(artists,i));
                 i++)
            {
                if (IsComposer_(artist, album_credits)) num_composers++;
            }

            // if more than a composer listed, multiple composers in the album
            if (num_composers>1) various_composers = true;

            // if only composer names are given
            if (num_artists==num_composers) various_performers = true;
        }
    }
}
