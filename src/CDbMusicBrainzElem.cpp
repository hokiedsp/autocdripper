#include "CDbMusicBrainzElem.h"

#include <utility>
#include <stdexcept>
#include <cstdlib>
#include <climits>
#include <cctype>
//#include <ctime>
//#include <sstream>
//#include <iomanip>
//#include <numeric> // for std::accumulate

#include <iostream>
using std::cout;
using std::endl;

#include "utils.h"

CDbMusicBrainzElem::CDbMusicBrainzElem(const std::string &rawdata, const int d)
    : CUtilXmlTree(rawdata), disc(d)
{
    // change the root to release element
    if (root)
    {
        if (!FindObject("release",root) && root)
            throw(std::runtime_error("MusicBrainz release lookup resulted in an invalid result."));

        AnalyzeArtists_();
    }
}

/**
 * @brief Exchanges the content with another CDbMusicBrainzElem object
 * @param Another CDbMusicBrainzElem object
 */
void CDbMusicBrainzElem::Swap(CDbMusicBrainzElem &other)
{
    CUtilXmlTree::Swap(other);
    std::swap(disc,other.disc);
    artists.swap(other.artists);
}

/**
 * @brief Load New Data. Clears existing data first. If empty, no action.
 * @param valid XML string following musicbrainz_mmd-2.0.rng
 * @throw runtime_error if invalid XML string is passed in.
 */
void CDbMusicBrainzElem::LoadData(const std::string &rawdata)
{
    // call the base class function first
    CUtilXmlTree::LoadData(rawdata);

    // change the root to release element
    if (root)
    {
        if (!FindObject("release",root) && root)
        throw(std::runtime_error("MusicBrainz release lookup resulted in an invalid result."));

        AnalyzeArtists_();
    }
}

/**
 * @brief Clear existing XML data
 */
void CDbMusicBrainzElem::ClearData()
{
    disc = 0;
    artists.clear();
}

// -----------------------------------------------------------------------------------------

/** Return a unique release ID string
 *
 *  @return id string if Query was successful.
 */
std::string CDbMusicBrainzElem::ReleaseId() const
{
    std::string rval;
    FindElementAttribute(root,"id",rval);
    return rval;
}

/** Get album title
 *
 *  @return    Title string (empty if title not available)
 */
std::string CDbMusicBrainzElem::AlbumTitle() const
{
    std::string rval;
    FindString("title",rval);
    return rval;
}

/** Get album artist
 *
 *  @return    Artist string (empty if artist not available)
 */
std::string CDbMusicBrainzElem::AlbumArtist() const
{
    return Artists_(root,false);
}

/** Get album composer
 *
 *  @return    Composer/songwriter string (empty if artist not available)
 */
std::string CDbMusicBrainzElem::AlbumComposer() const
{
    return Artists_(root,true);
}

/** Get release date
 *
 *  @return    Date string (empty if genre not available)
 */
std::string CDbMusicBrainzElem::Date() const
{
    std::string str;
    FindString("date",str);
    return str;
}

/** Get release country
 *
 *  @return    Countery string (empty if genre not available)
 */
std::string CDbMusicBrainzElem::Country() const
{
    std::string str;
    FindString("country",str);
    return str;
}

/**
 * @brief Get disc number
 * @return    Disc number or -1 if unknown
 */
int CDbMusicBrainzElem::DiscNumber() const
{
    return disc;
}

/**
 * @brief Get total number of discs in the release
 * @return    Number of discs or -1 if unknown
 */
int CDbMusicBrainzElem::TotalDiscs() const
{
    int rval = 1;
    const xmlNode *medium;
    FindArray("medium-list", medium, &rval);

    return rval;
}

/** Get label name
 *
 *  @return    Label string (empty if label not available)
 */
std::string CDbMusicBrainzElem::AlbumLabel() const
{
    std::string rval;

    const xmlNode *labelinfo, *label;
    if (FindArray("label-info-list",labelinfo) && labelinfo
            && FindObject(labelinfo,"label",label) && label)
        FindString(label, "name",rval);

    return rval;
}

/** Get catalog number
 *
 *  @return    CatNo string (empty if not available)
 */
std::string CDbMusicBrainzElem::AlbumCatNo() const
{
    std::string rval;

    const xmlNode *labelinfo;
    if (FindArray("label-info-list",labelinfo) && labelinfo)
        FindString(labelinfo, "catalog-number",rval);

    return rval;
}

/** Get album UPC
 *
 *  @return    UPC string (empty if UPC not available)
 */
std::string CDbMusicBrainzElem::AlbumUPC() const
{
    std::string str;
    FindString("barcode",str);
    return str;
}

/** Get Amazon Standard Identification Number
 *
 *  @return    ASIN string (empty if ASIN not available)
 */
std::string CDbMusicBrainzElem::AlbumASIN() const
{
    std::string str;
    FindString("asin",str);
    return str;
}

/**
 * @brief Find XML object for the medium associated with the CD
 * @return pointer to a medium XML object
 */
const xmlNode* CDbMusicBrainzElem::GetMedium_() const
{
    int pos;
    const xmlNode *medium = NULL;
    for(FindArray("medium-list",medium);
        medium && FindInt(medium,"position",pos) && pos!=disc;
        medium = medium->next);
    return medium;
}

/**
 * @brief Find JSON object for the specified track
 * @param[in] track number counting over multiple discs
 * @param[out] if not NULL and track is found in sub_tracks listing, returns its parent index track JSON object
 * @return pointer to a track JSON object
 */
const xmlNode* CDbMusicBrainzElem::GetTrack_(const size_t tracknum) const
{
    const xmlNode *medium = GetMedium_();
    const xmlNode *track = NULL;
    if (medium && FindArray(medium,"track-list",track))
    {
        int pos;
        while (track && FindInt(track,"position",pos) && pos!=(int)tracknum)
            track = track->next;
    }
    return track;
}


/** Get number of tracks
 *
 *  @return    number of tracks
 *  @throw     runtime_error if CD record id is invalid
 */
int CDbMusicBrainzElem::NumberOfTracks() const
{
    int rval;
    const xmlNode* medium = GetMedium_();
    const xmlNode *track;
    if (medium) FindArray(medium,"track-list",track,&rval);
    return rval;
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
std::string CDbMusicBrainzElem::TrackTitle(int tracknum) const
{
    std::string rval;
    const xmlNode *track = GetTrack_(tracknum);
    const xmlNode *recording;

    // look on the track first, if not found, check in its recording
    if (track && !FindString(track,"title",rval) && FindObject(track,"recording",recording))
        FindString(recording,"title",rval);

    return rval;
}

/** Get track artist
 *
 *  @param[in] Track number (1-99)
 *  @return    Artist string (empty if artist not available)
 *  @throw     runtime_error if track number is invalid
 */
std::string CDbMusicBrainzElem::TrackArtist(int tracknum) const
{
    std::string rval;
    const xmlNode *track = GetTrack_(tracknum);
    const xmlNode *recording;

    if (FindObject(track,"recording",recording))
    {
        rval = Artists_(recording, false);
    }
    else    // only if Recording artists are not available, look up track artists
    {
        rval = Artists_(track,false);
    }

    return rval;
}

/** Get track composer
 *
 *  @param[in] Track number (1-99)
 *  @return    Composer string (empty if artist not available)
 *  @throw     runtime_error if track number is invalid
 */
std::string CDbMusicBrainzElem::TrackComposer(int tracknum) const
{
    const xmlNode *track = GetTrack_(tracknum);
    return Artists_(track,true);
}

/** Get track ISRC
 *
 *  @param[in] Track number (1-99)
 *  @return    ISRC string
 *  @throw     runtime_error if track number is invalid
 */
std::string CDbMusicBrainzElem::TrackISRC(int tracknum) const
{
    std::string rval;
    const xmlNode *recording, *isrc;
    const xmlNode *track = GetTrack_(tracknum);

    if (track && FindObject(track,"recording",recording) &&
            FindArray(recording,"isrc-list",isrc) && isrc)
        FindElementAttribute(isrc,"id",rval);

    return rval;
}

/** Get a vector of track lengths
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Vector of track lengths in seconds
 *  @throw     runtime_error if release number is invalid
 */
std::vector<int> CDbMusicBrainzElem::TrackLengths() const
{
    std::vector<int> tracklengths;

    const xmlNode *medium;
    const xmlNode *track = NULL;
    int len, pos, count;

    medium = GetMedium_();
    if (!FindArray(medium, "track-list", track, &count))
        throw(std::runtime_error("Failed to obtain the track list."));

    // preallocate the vector to the expected size
    tracklengths.reserve(count);

    for (; track; track = track->next)
    {
        if (!FindInt(track,"lnegth",len))
            throw(std::runtime_error("Failed to obtain a track length."));

        if (FindInt(track,"position",pos)) tracklengths[pos-1] = len;
        else tracklengths.push_back(len);
    }

    return tracklengths;
}

/**
 * @brief Get a related URL.
 * @param[in]  Relationship type (e.g., "discogs", "amazon asin")
 * @return URL string or empty if requestd URL type not in the URL
 */
std::string CDbMusicBrainzElem::RelationUrl(const std::string &type) const
{
    std::string rval;
    bool relation_notfound=true, target_notfound = true;
    const xmlNode *list, *relation;
    std::string target;

    // look through all relation
    if (FindObject("relation-list", list))
    {
        do
        {
            if (FindElementAttribute(list,"target-type",target) && target.compare("url")==0)
            {
                relation_notfound = false;
                for (relation = list->children;
                     relation && (target_notfound = CompareElementAttribute(relation,"type",type)!=0);
                     relation = relation->next);

                if (!target_notfound) FindString(relation,"target",rval);
            }
        } while (relation_notfound && FindNextElement(list, "relation-list", list));
    }

    return rval;
}

/**
 * @brief Get associated ReleaseGroup ID
 * @return Release group ID
 */
std::string CDbMusicBrainzElem::ReleaseGroupId() const
{
    std::string rval;
    const xmlNode *rgroup;
    FindObject("release-group", rgroup, &rval);
    return rval;
}

//-----------------------------------------------------------------------------

/** Check if the query returned a front cover
 *
 *  @return     true if front cover is found.
 */
bool CDbMusicBrainzElem::Front() const
{
    bool rval = false;
    const xmlNode *node;
    int val;
    if (FindObject("cover-art-archive",node) && FindInt(node,"front",val))
        rval = val;
    return rval;
}

/** Check if the query returned a back cover
 *
 *  @return     true if back cover is found.
 */
bool CDbMusicBrainzElem::Back() const
{
    bool rval = false;
    const xmlNode *node;
    int val;

    if (FindObject("cover-art-archive",node) && FindInt(node,"back",val))
        rval = val;
    return rval;
}

//-----------------------------------------------------------------------------

/**
 * @brief Generate artist map to identify if they are composer
 */
void CDbMusicBrainzElem::AnalyzeArtists_()
{
    std::string id;
    const xmlNode *medium, *track, *recording, *credit, *dummy;
    std::vector<std::string> track_artists;
    std::vector<std::string>::iterator it;
    typedef std::pair<std::string,bool> ArtistData;
    std::pair<std::map<std::string,bool>::iterator,bool> ret;

    // first look in album's artist-credit
    for (FindArray("artist-credit",credit); credit; credit = credit->next)
    {
        // Add ID to the artist map as non-composer
        if (FindObject(credit, "artist", dummy, &id))
            artists.insert(std::pair<std::string,bool>(id,false));
    }

    // now look in track & recording's artist-credits
    medium = GetMedium_();
    for (FindArray(medium,"track-list",track); track; track=track->next)
    {
        // get track artists and temporarily store it in a vector
        track_artists.clear();
        for (FindArray(track, "artist-credit", credit); credit; credit=credit->next)
        {
            // Add ID to the artist map as non-composer
            if (FindObject(credit, "artist", dummy, &id))
                track_artists.push_back(id);
        }

        // now get the recording artists
        if (FindObject(track,"recording",recording))
        {
            for (FindArray(recording,"artist-credit", credit); credit; credit = credit->next)
            {
                if (FindObject(credit, "artist", dummy, &id))
                {
                    // add the recording artist as non-composer
                    artists.insert(ArtistData(id,false));

                    // check if track artist is also a recording artist
                    bool iscomposer = true;
                    for (it=track_artists.begin(); iscomposer && it!=track_artists.end(); it++)
                    {
                        if (id.compare(*it)==0) // match found -> non-composer
                        {
                            // remove from track list
                            track_artists.erase(it);
                            iscomposer = false;
                        }
                    }
                }
            }

            // remaining track artists are treated as composer
            for (it=track_artists.begin(); it!=track_artists.end(); it++)
            {
                ret = artists.insert(ArtistData(*it,true));
                if (!ret.second) // data aready exists, mark it composer
                    (*(ret.first)).second = true;
            }
        }
        else // if no recording given, assume non-composer
        {
            for (it=track_artists.begin(); it!=track_artists.end(); it++)
                artists.insert(ArtistData(id,false));
        }
    }
}

std::string CDbMusicBrainzElem::Artists_(const xmlNode *node, const bool reqcomposer) const // maybe release or track json_t
{
    std::string rval;
    std::string name, joinstr, id;

    const xmlNode *credit, *artist;
    bool iscomposer;

    // first look in album's artist-credit
    for (FindArray(node, "artist-credit", credit); credit; credit = credit->next)
    {
        // Add ID to the artist map as non-composer
        if (FindObject(credit, "artist", artist, &id))
        {
            // check for composer/non-composer condition
            iscomposer = artists.at(id); // composer
            if (((iscomposer && reqcomposer) || (!(iscomposer || reqcomposer)))
                    && FindString(artist, "name", name)) // get the name
            {
                // if there is a joining string carried over from the previous artist, add now
                if (joinstr.size()) rval += joinstr;
                rval += name;

                // save its joining string for the next artist
                FindElementAttribute(credit,"joinphrase",joinstr);
            }
        }
    }

    return rval;
}

bool CDbMusicBrainzElem::FindObject(const xmlNode *parent, const std::string &name, const xmlNode *&node, std::string *id)
{
    // find a node with requested name
    bool rval = FindElement(parent, name, node);

    // if found and its ID URI is requested, try retrieving it (returns "" if does not exist)
    if (rval && id) FindElementAttribute(node, "id", *id);

    return rval;
}

bool CDbMusicBrainzElem::FindArray(const xmlNode *parent, const std::string &name, const xmlNode *&node, int *count, int *offset)
{
    // find a node with requested name
    bool rval = CUtilXmlTree::FindArray(parent, name, node, NULL);

    if (rval && (count||offset))   // found && array size requested
    {

        const xmlNode *array = node->parent;
        std::string attr_val;

        if (count)
        {
            FindElementAttribute(array, "count", attr_val);
            try
            {
                *count = std::stoi(attr_val);
            }
            catch(...)  // if attribute does not exist or not integer, count the # of children
            {
                *count = xmlChildElementCount(const_cast<xmlNode*>(array));
            }
        }

        if (offset)
        {
            FindElementAttribute(array, "offset", attr_val);
            try
            {
                *offset = std::stoi(attr_val);
            }
            catch(...)  // if attribute does not exist or not integer, count the # of children
            {
                *offset = -1; // unknown
            }
        }
    }

    return rval;
}
