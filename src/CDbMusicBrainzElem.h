#pragma once

#include <vector>
#include <map>
#include <string>

#include "CDbMusicBrainzElemBase.h"
//#include "utils.h"

class CDbMusicBrainz;

class CDbMusicBrainzElem : protected CDbMusicBrainzElemBase
{
    friend class CDbMusicBrainz;
public:
    CDbMusicBrainzElem(const std::string &data, const int disc=1);
    virtual ~CDbMusicBrainzElem() {}

    /**
     * @brief Exchanges the content with another CDbMusicBrainzElem object
     * @param Another CDbMusicBrainzElem object
     */
    virtual void Swap(CDbMusicBrainzElem &other);

    /**
     * @brief Load New Data. Clears existing data first. If empty, no action.
     * @param valid XML string following musicbrainz_mmd-2.0.rng
     * @throw runtime_error if invalid XML string is passed in.
     */
    virtual void LoadData(const std::string &data);

    /**
     * @brief Clear existing XML data
     */
    virtual void ClearData();

    /** Return a unique release ID string
     *
     *  @return id string if Query was successful.
     */
    std::string ReleaseId() const;

    /** Get album title
     *
     *  @return    Title string (empty if title not available)
     */
    std::string AlbumTitle() const;

    /** Get album artist
     *
     *  @return    Artist string (empty if artist not available)
     */
    std::string AlbumArtist() const;

    /** Get album composer
     *
     *  @return    Composer/songwriter string (empty if artist not available)
     */
    std::string AlbumComposer() const;

    /** Get genre
     *
     *  @return    Genre string (empty if genre not available)
     */
    std::string Genre() const;

    /** Get release date
     *
     *  @return    Date string (empty if genre not available)
     */
    std::string Date() const;

    /** Get release country
     *
     *  @return    Countery string (empty if genre not available)
     */
    std::string Country() const;

    /**
     * @brief Get disc number
     * @return    Disc number or -1 if unknown
     */
    int DiscNumber() const;

    /**
     * @brief Get total number of discs in the release
     * @return    Number of discs or -1 if unknown
     */
    int TotalDiscs() const;

    /** Get label name
     *
     *  @return    Label string (empty if label not available)
     */
    std::string AlbumLabel() const;

    /** Get catalog number
     *
     *  @return    Catalog Number string (empty if not available)
     */
    std::string AlbumCatNo() const;

    /** Get album UPC
     *
     *  @return    UPC string (empty if UPC not available)
     */
    std::string AlbumUPC() const;

    /** Get number of tracks
     *
     *  @return    number of tracks
     *  @throw     runtime_error if CD record id is invalid
     */
    int NumberOfTracks() const;

    /** Get track title
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
    std::string TrackTitle(int tracknum) const;

    /** Get track artist
     *
     *  @param[in] Track number (1-99)
     *  @return    Artist string (empty if artist not available)
     *  @throw     runtime_error if track number is invalid
     */
    std::string TrackArtist(int tracknum) const;

    /** Get track composer
     *
     *  @param[in] Track number (1-99)
     *  @return    Composer string (empty if artist not available)
     *  @throw     runtime_error if track number is invalid
     */
    std::string TrackComposer(int tracknum) const;

    /** Get track ISRC
     *
     *  @param[in] Track number (1-99)
     *  @return    ISRC string
     *  @throw     runtime_error if track number is invalid
     */
    std::string TrackISRC(int tracknum) const;

    /** Get a vector of track lengths
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Vector of track lengths in seconds
     *  @throw     runtime_error if release number is invalid
     */
    std::vector<int> TrackLengths() const;

    /**
     * @brief Get a related URL.
     * @param[in]  Relationship type (e.g., "discogs", "amazon asin")
     * @return URL string or empty if requestd URL type not in the URL
     */
    std::string RelationUrl(const std::string &type) const;

    /**
     * @brief Get associated ReleaseGroup ID
     * @return Release group ID
     */
    std::string ReleaseGroupId() const;

    //---------------------------------------------------------

    /** Check if the query returned a front cover
     *
     *  @return     true if front cover is found.
     */
    bool Front() const;

    /** Check if the query returned a back cover
     *
     *  @return     true if back cover is found.
     */
    bool Back() const;

private:
    int disc; // in the case of multi-disc set, indicate the disc # (zero-based)
    std::map<std::string,bool> artists; // <MBID, false-artist/performer, true-composer>

    /**
     * @brief Collect all the artists appear in the album and identify if they are a composer
     */
    void AnalyzeArtists_();

    /**
     * @brief Find XML object for the medium associated with the CD
     * @return pointer to a medium XML object
     */
    const xmlNode* GetMedium_() const;

    /**
     * @brief Find JSON object for the specified track
     * @param[in] track number counting over multiple discs
     * @param[out] if not NULL and track is found in sub_tracks listing, returns its parent index track JSON object
     * @return pointer to a track JSON object
     */
    const xmlNode* GetTrack_(const size_t tracknum) const;

    std::string Artists_(const xmlNode *node, const bool reqcomposer) const; // maybe release or track json_t
};
