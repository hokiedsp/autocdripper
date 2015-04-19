#pragma once

#include <functional>
#include <vector>
#include <string>

#include "CUtilJson.h"
#include "SCueSheet.h"

class CDbDiscogsElem : protected CUtilJson
{
    friend class CDbDiscogs;
public:
    CDbDiscogsElem(const std::string &data, const int disc=1, const int offset=0);
    CDbDiscogsElem(const std::string &data, const int disc, const std::vector<int> &tracklengths);
    virtual ~CDbDiscogsElem() {}

    /**
     * @brief Exchanges the content with another CDbDiscogsElem object
     * @param Another CDbDiscogsElem object
     */
    void Swap(CDbDiscogsElem &other);

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
    SCueArtists AlbumArtist() const;

    /** Get album composer
     *
     *  @return    Composer/songwriter string (empty if artist not available)
     */
    SCueArtists AlbumComposer() const;

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
    SCueArtists TrackArtist(int tracknum) const;

    /** Get track composer
     *
     *  @param[in] Track number (1-99)
     *  @return    Composer string (empty if artist not available)
     *  @throw     runtime_error if track number is invalid
     */
    SCueArtists TrackComposer(int tracknum) const;

    /** Get track ISRC
     *
     *  @param[in] Track number (1-99)
     *  @return    ISRC string
     *  @throw     runtime_error if track number is invalid
     */
    std::string TrackISRC(int tracknum) const;

    std::string Identifier(const std::string type) const;

private:
    int disc; // in the case of multi-disc set, indicate the disc # (zero-based)
    int track_offset; // starting track of the CD (always 0 if single disc release)
    int number_of_tracks; // number of tracks on the CD (-1 to use all tracks of the release)

    bool various_performers;    // false if album has unique set of performing artists
    bool various_composers;     // false if album has a single composer
    json_t *album_credits;      // fixed link to the release's extraartists

    /**
     * @brief Traverses tracklist array and calls Callback() for every track-type element
     * @param[in] Callback function
     *
     * Callback (lambda) function:
     * - Called on every JSON object under tracklist" with its "type_"=="track"
     * - Input arguments:
     *   track: Pointer to the current JSON "track" object
     *   parent: Pointer to the parent JSON "index track" object if track is a sub_track. Or
     *           NULL if track is not a sub_track
     *   pidx: sub_track index if parent is not NULL. Otherwise, unknown
     *   heading: Pointer to the last seen JSON "heading track" object. NULL if there has been none.
     *   hidx: track index (only counting the main tracks) w.r.t. heading. Unknonwn if heading is NULL,
     */
    void TraverseTracks_(
            std::function<bool (const json_t *track,
                                const json_t *parent, size_t pidx,
                                const json_t *heading, size_t hidx)> Callback) const;

    /**
     * @brief Find JSON object for the specified track
     * @param[in] track number counting over multiple discs
     * @param[out] if not NULL and track is found in sub_tracks listing, returns its parent index track JSON object
     * @return pointer to a track JSON object
     */
    const json_t* FindTrack_(const size_t trackno, const json_t **index=NULL, size_t *suboffset=NULL,
                             const json_t **header=NULL, size_t *headoffset=NULL) const;

    /**
     * @brief Determine track offset for multi-disc release
     *
     * This function fills track_offset & number_of_tracks member variables.
     *
     * @param[in] vector of lengths of CD tracks in seconds
     * @return false if Discogs release is invalid
     */
    bool SetDiscOffset_(const std::vector<int> &tracklengths);

    /**
     * @brief Fill number_of_tracks
     */
    void SetDiscSize_();

    /**
     * @brief return the total number of sub_tracks listed under an index track
     * @param[in] pointer to the index track JSON object
     * @return number of sub_tracks or 0 if failed
     */
    static size_t NumberOfSubTracks_(const json_t *track);

    /**
     * @brief Get title of album/track
     * @param[in] pointer to either album or track JSON object
     * @return the title of album or track
     */
    static std::string Title_(const json_t* data); // maybe release or track json_t

    /**
     * @brief Get a string of performer names associated with album or track. It excludes
     *        composer names
     * @param[in] pointer to album or track JSON object
     * @return string of performers
     */
    static SCueArtists Performer_(const json_t* data); // maybe release or track json_t

    /**
     * @brief Get a string of performer names associated with album or track. It excludes
     *        composer names
     * @param[in] pointer to album or track JSON object
     * @return composer
     */
    SCueArtists Composer_(const json_t* data) const; // maybe release or track json_t

    /**
     * @brief Analyze Artists lists to fill various_performers, various_composers, and album_credits
     */
    void AnalyzeArtists_();

    static bool IsComposer_(const json_t* artist, const json_t* extraartists,
                            const std::vector<std::string> &keywords
                            = {"Composed", "Written", "Adapted",
                               "Arranged", "Lyrics", "Music", "Songwriter",
                               "Words", "Orchestrated"});

    /**
     * @brief Get artist's Discogs ID
     * @param artist JSON object (an element of artists or extraartists arrays)
     * @return integer ID
     */
    static json_int_t ArtistId_(const json_t* artist);

    /**
     * @brief Look for the artist with the ID in the array of artists
     * @param id
     * @param artists
     * @return JSON object corresponds to the artist under search or NULL if not found
     */
    static json_t *FindArtist_(const json_int_t id, const json_t* artists);
};
