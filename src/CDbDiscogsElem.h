#pragma once

#include "CDbElemJsonBase.h"

class CDbDiscogsElem : public CDbElemJsonBase
{
public:
    CDbDiscogsElem(const std::string &data, const int disc=1);
    virtual ~CDbDiscogsElem() {}

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

    json_t* TrackList_() const;
    static std::string Title_(const json_t* data); // maybe release or track json_t
    static std::string Artist_(const json_t* data, const int artisttype=0); // maybe release or track json_t
};
