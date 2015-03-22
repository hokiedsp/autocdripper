#pragma once

#include <string>
#include <curl/curl.h>
#include <jansson.h>

#include "IDatabase.h"
#include "IReleaseDatabase.h"
#include "IImageDatabase.h"
#include "CDbBase.h"
#include "CDbHttpBase.h"
#include "CDbJsonBase.h"
#include "SDbrBase.h"

struct SCueSheet;

/** Discogs CD Database Record structure - SCueSheet with DbType()
 */
struct SDbrDiscogs : SDbrBase
{
    /** Name of the database the record was retrieved from
   *
   *  @return Name of the database
   */
    virtual std::string SourceDatabase() const { return "Discogs"; }

    friend std::ostream& operator<<(std::ostream& stdout, const SDbrDiscogs& obj);
};

/** Overloaded stream insertion operator to output the content of SDbrDiscogs
 *  object. The output is in accordance with the CDRWIN's Cue-Sheet syntax
 *
 *  @param[in]  Reference to an std::ostream object
 *  @return     Copy of the stream object
 */
inline std::ostream& operator<<(std::ostream& os, const SDbrDiscogs& o)
{
    os << dynamic_cast<const SCueSheet&>(o);
    return os;
}

class CDbMusicBrainz;

/** Class to access Discogs online CD databases service.
 */
class CDbDiscogs :
        public IDatabase, public IReleaseDatabase, public IImageDatabase,
        public CDbHttpBase, public CDbJsonBase
{
public:
    /** Constructor.
     *
     *  @param[in] Client program name. If omitted or empty, uses "autorip"
     *  @param[in] Client program version. If omitted or empty, uses "alpha"
     */
    CDbDiscogs(const std::string &cname="autorip",const std::string &cversion="alpha");

    /** Constructor.
     *
     *  @param[in] Client program name. If omitted or empty, uses "autorip"
     *  @param[in] Client program version. If omitted or empty, uses "alpha"
     */
    CDbDiscogs(const CDbMusicBrainz &mb, const std::string &cname="autorip",const std::string &cversion="alpha");

    /** Destructor
     */
    virtual ~CDbDiscogs();

    /**
     * @brief Return true if Database can be searched for a CD info
     * @return true if database can be searched for a CD release
     */
    bool IsReleaseDb() { return true; }

    /**
     * @brief Return true if Database can be searched for a CD cover art
     * @return true if database can be searched for a CD cover art
     */
    bool IsImageDb() { return false; } // for now (depends on Authentification)

    /**
     * @brief Return true if DB depends on MusicBrainz results
     * @return
     */
    bool DependsOnMusicBrainz() { return true; }

    /**
     * @brief Return database type enum
     * @return ReleaseDatabase enumuration value
     */
    virtual ReleaseDatabase GetDatabaseType() const { return ReleaseDatabase::DISCOGS; }

    /** Returns false as Discogs database cannot be queried based on CD track info.
     *
     *  @return    true if query is supported
     */
    virtual bool IsQueryable() const { return false; }

    /** Returns true as Discogs database can be searched by album title and artist.
     *
     *  @return    true if search is supported
     */
    virtual bool IsSearchable() const { return true; }

    /** Discogs does not support direct CD query. Always returns 0.
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

    /** Perform a new Discogs query given a list of its release IDs
     *
     *  @param[in] List of release IDs
     *  @return    Number of valid records
     */
    virtual int Query(const std::deque<std::string> &list);

    /** Perform a new Discogs query given a release ID and disc #
     *
     *  @param[in] List of release IDs
     *  @param[in] If multi-disc set, specifies the disc#
     *  @return    Number of valid records (1)
     */
    virtual int Query(const std::string &id, const int disc=1);

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

    /** Return the Discogs discid string
     *
     *  @return Discogs discid string if Query was successful. Otherwise "00000000".
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
     *  @return    Empty string (Discogs does not support genre)
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
    virtual std::string AlbumLabel(const int recnum=0) const;

    /** Get album UPC
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    UPC string (empty if title not available)
     */
    virtual std::string AlbumUPC(const int recnum=-1) const;

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
     * @brief Get number of discs in the release
     * @param Release record ID (0-based index). If omitted, the first record (0)
     *        is returned.
     * @return Number of discs in the release
     */
    int NumberOfDiscs(const int recnum=0) const;

private:
    static const std::string base_url;

    std::deque<int>discnos; // in the case of multi-disc set, indicate the disc # (zero-based)

    /** Goes through OAuth authorization procedure. Will fork to show user authorization webpage.
     *  Authorize_() will wait for the generated token to be copied back.
     * @brief
     */
    void Authorize_();

    static std::string Title_(const json_t* data); // maybe release or track json_t
    static std::string Artist_(const json_t* data); // maybe release or track json_t
    static std::string Genre_(const json_t* release);
    static std::string Label_(const json_t* release);
    static std::string Date_(const json_t* release);
    static std::string Country_(const json_t* release);
    static std::string Identifier_(const json_t* release, const std::string type);
    static int NumberOfDiscs_(const json_t *release);
    static json_t* TrackList (const json_t *release, const int discno);
};
