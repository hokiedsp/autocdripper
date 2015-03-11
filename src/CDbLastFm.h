#pragma once

#include <string>
#include <curl/curl.h>
#include <jansson.h>

#include "CDbHttpBase.h"
#include "CDbJsonBase.h"
#include "ICoverArt.h"
#include "SDbrBase.h"

struct SCueSheet;

/** Last.fm Database CD Record structure - SCueSheet with DbType()
 */
struct SDbrLastFm : SDbrBase
{
    /** Name of the database the record was retrieved from
   *
   *  @return Name of the database
   */
    virtual std::string SourceDatabase() const { return "LastFm"; }

    friend std::ostream& operator<<(std::ostream& stdout, const SDbrLastFm& obj);
};

/** Overloaded stream insertion operator to output the content of SDbrLastFm
 *  object. The output is in accordance with the CDRWIN's Cue-Sheet syntax
 *
 *  @param[in]  Reference to an std::ostream object
 *  @return     Copy of the stream object
 */
inline std::ostream& operator<<(std::ostream& os, const SDbrLastFm& o)
{
    os << dynamic_cast<const SCueSheet&>(o);
    return os;
}

class CDbMusicBrainz;

/** Class to access last.fm online CD databases service.
 */
class CDbLastFm : public CDbHttpBase, public CDbJsonBase
{
public:
    /** Constructor.
     *
     *  @param[in] Client program last.fm API key
     *  @param[in] Client program name. If omitted or empty, uses "autorip"
     *  @param[in] Client program version. If omitted or empty, uses "alpha"
     */
    CDbLastFm(const std::string &apikey, const std::string &cname="autorip", const std::string &cversion="alpha");

    /** Constructor.
     *
     *  @param[in] Client program last.fm API key
     *  @param[in] Client program name. If omitted or empty, uses "autorip"
     *  @param[in] Client program version. If omitted or empty, uses "alpha"
     */
    CDbLastFm(const CDbMusicBrainz &mb, const std::string &apikey, const std::string &cname="autorip",const std::string &cversion="alpha");

    /** Destructor
     */
    virtual ~CDbLastFm();

    /** Returns false as LastFm database cannot be queried based on CD track info.
     *
     *  @return    true if query is supported
     */
    virtual bool IsQueryable() const { return false; }

    /** Returns true as LastFm database can be searched by album title and artist.
     *
     *  @return    true if search is supported
     */
    virtual bool IsSearchable() const { return true; }

    /** LastFm does not support direct CD query. Always returns 0.
     *
     *  @param[in] CD-ROM device path
     *  @param[in] Cuesheet with its basic data populated (not used)
     *  @param[in] Length of the CD in seconds (not used)
     *  @param[in] If true, immediately calls Read() to populate disc records.
     *  @param[in] Network time out in seconds. (not used)
     *  @return    Number of matched records
     */
    virtual int Query(const std::string &dev, const SCueSheet &cuesheet, const size_t len, const bool autofill=false, const int timeout=-1)
    { return 0; }

    /** Perform a new LastFm query given a list of its release IDs
     *
     *  @param[in] List of MusicBrainz release IDs
     *  @return    Number of valid records
     */
    virtual int Query(const std::deque<std::string> &list);

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
    virtual int Search(const std::string &title, const std::string &artist, const bool autofill=false, const int timeout=-1);

    /** Return the LastFm discid string
     *
     *  @return LastFm discid string if Query was successful. Otherwise "00000000".
     */
    virtual std::string GetDiscId() const { return ""; }

    /** Get album title
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Title string (empty if title not available)
     */
    virtual std::string AlbumTitle(const int recnum=-1) const;

    /** Get album artist
         *
         *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
         *             is returned.
         *  @return    Artist string (empty if artist not available)
         */
    virtual std::string AlbumArtist(const int recnum=-1) const;

    /** Get genre
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Empty string (LastFm does not support genre)
     */
    virtual std::string Genre(const int recnum=0) const;

    /**
     * @brief Returns number of tracks on the CD
     * @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     * @return Number of tracks
     */
    virtual int NumberOfTracks(const int recnum=0) const;

    /** Get label name
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Label string (empty if label not available)
     */
    virtual std::string AlbumLabel(const int recnum=0) const { return ""; }

    /** Get album UPC
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    UPC string (empty if title not available)
     */
    virtual std::string AlbumUPC(const int recnum=-1) const { return ""; }

    /** Returns the CD record ID associated with the specified genre. If no matching
     *  record is found, it returns -1.
     *
     *  @return Matching CD record ID.
     */
    virtual int MatchByGenre(const std::string &genre) const;

    /** No action performed as the full data is retrieved by query/search call.
     *
     *  @param[in] Disc record ID (0-based index to discs). If negative, retrieves info
     *             for all records.
     *  @param[in] Network time out in seconds. If omitted or negative, previous value
     *             will be reused. System default is 10.
     */
    virtual void Populate(const int discnum=-1, const int timeout=-1) {}

    /** Retrieve the disc info from specified database record
         *
         *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
         *             is returned.
         *  @return    SDbrBase Pointer to newly created database record object. Caller is
         *             responsible for deleting the object.
         */
    virtual SDbrBase* Retrieve(const int recnum=0) const;

    /**
     * @brief last.fm cannot resolve multi-disc entry. Always returns 1
     * @param Release record ID (0-based index). If omitted, the first record (0)
     *        is returned.
     * @return Always 1
     */
    int NumberOfDiscs(const int recnum=0) const { return 1; }

    /** Specify the preferred coverart image width
     *
     *  @param[in] Preferred width of the image
     */
    virtual void SetPreferredWidth(const size_t &width);

    /** Specify the preferred coverart image height
     *
     *  @param[in] Preferred height of the image
     */
    virtual void SetPreferredHeight(const size_t &height);

    /** Check if the query returned a front cover
     *
     *  @param[in]  record index (default=0)
     *  @return     true if front cover is found.
     */
    virtual bool Front(const int recnum=0) const;

    /** Check if the query returned a back cover
     *
     *  @param[in]  record index (default=0)
     *  @return     true if back cover is found.
     */
    virtual bool Back(const int recnum=0) const { return false; }

    /** Retrieve the front cover data.
     *
     *  @param[out] image data buffer.
     *  @param[in]  record index (default=0)
     */
    virtual std::vector<unsigned char> FrontData(const int recnum=0);

    /** Check if the query returned a front cover
     *
     *  @param[out] image data buffer.
     *  @param[in]  record index (default=0)
     */
    virtual std::vector<unsigned char> BackData(const int recnum=0) { std::vector<unsigned char> data; return data; }

    /** Get the URL of the front cover image
     *
     *  @param[in]  Record index (default=0)
     *  @return     URL string
     */
    virtual std::string FrontURL(const int recnum=0) const;

    /** Get the URL of the back cover image
     *
     *  @param[in]  Record index (default=0)
     *  @return     URL string
     */
    virtual std::string BackURL(const int recnum=0) const { return ""; }

private:
    static const std::string base_url;
    std::string apikey;
    int CoverArtSize; // 0-"small", 1-"medium", 2-"large", 3-"extralarge","mega"

    /**
     * @brief Form URL from MusicBrainz release ID
     * @param[in] MusicBrainz release ID
     * @param[in] last.fm API Key
     * @return Generated URL
     */
    static std::string FormUrlFromMbid(const std::string& MBID, const std::string &apikey);

    static std::string Title_(const json_t* data); // maybe release or track json_t
    static std::string AlbumArtist_(const json_t *release);
    static std::string TrackArtist_(const json_t *track);
    static std::string Genre_(const json_t *release);
    static std::string Date_(const json_t *release);
    static json_t* TrackList_(const json_t *release);

    static std::string ImageURL_(const json_t *release, const int CoverArtSize);
};
