#include "CDbDiscogsElem.h"

CDbDiscogsElem::CDbDiscogsElem(const std::string &data, const int d)
    : CUtilJson(data), disc(d)
{}

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
    json_t *array, *elem;
    size_t index;
    std::string str;
    int rval = -1;

    // look for format_quantity
    json_int_t nodiscs;
    if (FindInt("format_quantity", nodiscs))
    {
        rval = (int)nodiscs;
    }
    else
    {
        // look for qty entry under formats
        if (FindArray("formats",array))
        {
            // Look for format entries
            json_array_foreach(array, index, elem)
            {
                if (CompareString(elem,"name","CD") && FindString(elem,"qty",str))
                {
                    try
                    {
                        rval = std::stoi(str); // if qty given as a number, return it
                        break;
                    }
                    catch(...)
                    {}
                }
            }
        }

        // if for some reason, formats/qty is not found, check for existence of sub track field
        {
            if (FindArray("tracklist",array))
            {
                // If all tracks contain sub_tracks, assume multi-CD release, else single-cd release
                json_array_foreach(array, index, elem)
                {
                    if (!FindArray(elem,"sub_tracks",elem))
                    {
                        rval = 1;
                        break;
                    }
                }

                // If reached here, all tracks contains sub_tracks -> multi-cd release
                if (rval<1) json_array_size(array); // return # of tracks as the # of cds
            }
        }
    }

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
    return Identifier("Barcode");
}

/** Get number of tracks
 *
 *  @return    number of tracks
 *  @throw     runtime_error if CD record id is invalid
 */
int CDbDiscogsElem::NumberOfTracks() const
{
    return json_array_size(TrackList_());
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
    json_t *tracks = TrackList_();

    // check track number
    if (tracknum<=0 || tracknum>(int)json_array_size(tracks))
        throw(std::runtime_error("Invalid CD Track Number."));

    json_t *track = json_array_get(tracks,--tracknum);
    rval = Title_(track);

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
    json_t *tracks = TrackList_();

    // check track number
    if (tracknum<=0 || tracknum>(int)json_array_size(tracks))
        throw(std::runtime_error("Invalid CD Track Number."));

    json_t *track = json_array_get(tracks,--tracknum);
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

json_t* CDbDiscogsElem::TrackList_() const
{
    json_t *tracks;

    if (!FindArray("tracklist", tracks))
        throw(std::runtime_error("Tracklist not found."));

    // if multi-disc set
    if (TotalDiscs()>1)
        FindArray(json_array_get(tracks,disc-1),"sub_tracks",tracks);

    return tracks;
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
