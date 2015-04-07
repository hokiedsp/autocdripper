#pragma once

#include <functional>
#include <vector>
#include "CUtilJson.h"

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

    std::string Identifier(const std::string type) const;

private:
    int disc; // in the case of multi-disc set, indicate the disc # (zero-based)
    int track_offset; // starting track of the CD (always 0 if single disc release)
    int number_of_tracks; // number of tracks on the CD (-1 to use all tracks of the release)

    /**
     * @brief Traverses tracklist array and calls Callback() for every track-type element
     * @param[in] Callback function taking a pointer to track (or subtrack) JSON element
     *            and its parent track (only for subtrack, NULL if element is track) and
     *            returns true to go to next track, false to quit traversing
     */
    void TraverseTracks_(std::function<bool (const json_t *track, const json_t *parent)>) const;

    /**
     * @brief Find JSON object for the specified track
     * @param[in] track number between 1 and 99 -> 0
     * @param[out] if not NULL and track is found in sub_tracks listing, returns its parent index track JSON object
     * @return pointer to a track JSON object
     */
    const json_t* FindTrack_(const size_t trackno, const json_t **index=NULL) const;

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

    static std::string Title_(const json_t* data); // maybe release or track json_t
    static std::string Artist_(const json_t* data, const int artisttype=0); // maybe release or track json_t
};
