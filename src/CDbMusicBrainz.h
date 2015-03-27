#pragma once

#include <deque>
#include <string>

#include <musicbrainz5/Query.h>
#include <musicbrainz5/ArtistCredit.h>
#include <coverart/CoverArt.h>

#include "IDatabase.h"
#include "IReleaseDatabase.h"
#include "IImageDatabase.h"
#include "SDbrBase.h"

/** MusicBrainz CD Database Record structure - SCueSheet with DbType()
 */
struct SDbrMusicBrainz : SDbrBase
{
    /** Name of the database the record was retrieved from
   *
   *  @return Name of the database
   */
    virtual std::string SourceDatabase() const { return "MusicBrainz"; }

    friend std::ostream& operator<<(std::ostream& stdout, const SDbrMusicBrainz& obj);
};

/** Overloaded stream insertion operator to output the content of SDbrMusicBrainz
 *  object. The output is in accordance with the CDRWIN's Cue-Sheet syntax
 *
 *  @param[in]  Reference to an std::ostream object
 *  @return     Copy of the stream object
 */
inline std::ostream& operator<<(std::ostream& os, const SDbrMusicBrainz& o)
{
    os << dynamic_cast<const SCueSheet&>(o);
    return os;
}

/** Class to access MusicBrainz online CD and coverart databases service.
 */
class CDbMusicBrainz : public IDatabase, public IReleaseDatabase, public IImageDatabase
{
public:
    /** Constructor.
     *
     *  @param[in] Server name. If omitted or empty, default server
     *             "musicbrainz.org" is used.
     *  @param[in] Server port. If omitted, default value is 80.
     *  @param[in] MusicBrainz account name for tagging. If omitted or empty, no
     *             action is taken.
     *  @param[in] MusicBrainz account password for tagging. If omitted or empty
     *             no action is taken.
     *  @param[in] Client program name. If omitted or empty, uses "autorip"
     *  @param[in] Client program version. If omitted or empty, uses "alpha"
     */
    CDbMusicBrainz(const std::string &servername="musicbrainz.org", const int serverport=80,
                   const std::string &username="", const std::string &password="",
                   const std::string &cname="autorip",const std::string &cversion="alpha");

    /** Destructor
     */
    virtual ~CDbMusicBrainz();

    /////////////////////////////////////////
    // for IDatabase

    /**
     * @brief Return true if Database can be searched for a CD info
     * @return true if database can be searched for a CD release
     */
    bool IsReleaseDb() { return true; }

    /**
     * @brief Return true if Database can be searched for a CD cover art
     * @return true if database can be searched for a CD cover art
     */
    bool IsImageDb() { return true; }

    /**
     * @brief Return database type enum
     * @return ReleaseDatabase enumuration value
     */
    virtual DatabaseType GetDatabaseType() const { return DatabaseType::MUSICBRAINZ; }

    /**
     * @brief Return true if database can be queried directly from CD info
     * @return true if database can receive CD info based query
     */
    virtual bool AllowQueryCD() { return true; }

    /**
     * @brief Return true if MusicBrainz database is known to contain
     *        a link to this database
     * @return true if release ID is obtainable from MusicBrainz
     */
    virtual bool MayBeLinkedFromMusicBrainz() { return false; }

    /**
     * @brief Return true if database supports UPC barcode search
     * @return true if database supports UPC barcode search
     */
    virtual bool AllowSearchByUPC() { return false; }

    /**
     * @brief Return true if database supports search by album artist and title
     * @return true if database supports search by album artist and title
     */
    virtual bool AllowSearchByArtistTitle() { return false; }

    /**
     * @brief Return true if database supports search by CDDBID
     * @return true if database supports search by CDDBID
     */
    virtual bool AllowSearchByCDDBID() { return false; }

    /**
     * @brief Return true if database supports search by MusicBrainz ID
     * @return true if database supports search by MusicBrainz ID
     */
    virtual bool AllowSearchByMBID() { return false; }

    /** If AllowQueryCD() returns true, Query() performs a new query for the CD info
     *  in the specified drive with its *  tracks specified in the supplied cuesheet
     *  and its length. Previous query outcome discarded. After disc and its tracks
     *  are initialized, CDDB disc ID is computed. If the computation fails, function
     *  throws an runtime_error.
     *
     *  @param[in] CD-ROM device path
     *  @param[in] Cuesheet with its basic data populated
     *  @param[in] Length of the CD in sectors
     *  @param[in] (Optional) UPC barcode
     *  @return    Number of matched records
     */
    virtual int Query(const std::string &dev, const SCueSheet &cuesheet, const size_t len, const std::string cdrom_upc="")=0;

    /** If MayBeLinkedFromMusicBrainz() returns true, Query() performs a new
     *  query based on the MusicBrainz query results.
     *
     *  @param[in] MusicBrainz database object.
     *  @param[in] (Optional) UPC barcode
     *  @return    Number of matched records
     */
    virtual int Query(const CDbMusicBrainz &mbdb, const std::string upc="") { return 0; }

    /**
     * @brief Clear all the matches from previous search
     */
    virtual void Clear();

    ///////////////////////////////////////////////////////////////////////////

    /** Return the discid string
   *
   *  @return discid string if Query was successful.
   */
    virtual std::string GetDiscId() const;

    /** Returns the number of matched records returned from the last Query() call.
   *
   *  @return    Number of matched records
   */
    virtual int NumberOfMatches() const;

    /////////////////////////////////////////
    // for IReleaseDatabase

    /** Look up full disc information from CDDB server. It supports single record or
   *  multiple records if multiple entries were found by Query(). If the computation
   *  fails, function throws an runtime_error.
   *
   *  @param[in] Disc record ID (0-based index to discs). If negative, retrieves info
   *             for all records.
   *  @param[in] Network time out in seconds. If omitted or negative, previous value
   *             will be reused. System default is 10.
   */
    virtual void Populate(const int recnum=-1);

    /** Get album title
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Title string (empty if title not available)
     */
    virtual std::string AlbumTitle(const int recnum=0) const;

    /** Get album artist
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Artist string (empty if artist not available)
     */
    virtual std::string AlbumArtist(const int recnum=0) const;

    /** Get album composer
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Composer/songwriter string (empty if artist not available)
     */
    virtual std::string AlbumComposer(const int recnum=0) const { return ""; }

    /** Get genre
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Empty string (MusicBrainz does not support genre)
     */
    virtual std::string Genre(const int recnum=0) const { return ""; }

    /** Get release date
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Date string (empty if genre not available)
     */
    virtual std::string Date(const int recnum=0) const;

    /** Get release country
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Countery string (empty if genre not available)
     */
    virtual std::string Country(const int recnum=0) const;

    /**
     * @brief Get disc number
     * @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *            is returned.
     * @return    Disc number or -1 if unknown
     */
    virtual int DiscNumber(const int recnum=0) const;

    /**
     * @brief Get total number of discs in the release
     * @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *            is returned.
     * @return    Number of discs or -1 if unknown
     */
    virtual int TotalDiscs(const int recnum=0) const;

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
    /** Get album artist
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Composer/songwriter string (empty if artist not available)
     */

    /** Get track title
     *
     *  @param[in] Track number (1-99)
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Title string (empty if title not available)
     *  @throw     runtime_error if track number is invalid
     */
    virtual std::string TrackTitle(const int tracknum, const int recnum=0) const;

    /** Get track artist
     *
     *  @param[in] Track number (1-99)
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Artist string (empty if artist not available)
     *  @throw     runtime_error if track number is invalid
     */
    virtual std::string TrackArtist(const int tracknum, const int recnum=0) const;

    /** Get track composer
     *
     *  @param[in] Track number (1-99)
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Composer string (empty if artist not available)
     *  @throw     runtime_error if track number is invalid
     */
    virtual std::string TrackComposer(const int tracknum, const int recnum=0) const { return ""; }

    /** Get track ISRC
     *
     *  @param[in] Track number (1-99)
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    ISRC string
     *  @throw     runtime_error if track number is invalid
     */
    virtual std::string TrackISRC(const int tracknum, const int recnum=0) const;

    /////////////////////////////////////////
    // for IImageDatabase

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
    virtual bool Back(const int recnum=0) const;

    /** Retrieve the front cover data.
     *
     *  @param[out] image data buffer.
     *  @param[in]  record index (default=0)
     */
    virtual std::vector<unsigned char> FrontData(const int recnum=0) const;

    /** Check if the query returned a front cover
     *
     *  @param[out] image data buffer.
     *  @param[in]  record index (default=0)
     */
    virtual std::vector<unsigned char> BackData(const int recnum=0) const;

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
    virtual std::string BackURL(const int recnum=0) const;

    /** Return the MusicBrainz release ID string
     *
     *  @return MusicBrainz release ID string if Query was successful. Otherwise "".
     */
    virtual std::string GetReleaseId(const int recnum=0) const;

    //////////////////////////////////////////
    // Member functions independent of interfaces

    /** Set a server connection protocol.
     *
     *  @param[in] Connection protocol ("cddbp","http", or "proxy"). If omitted
     *             or empty, 'cddbp' is used by the default. If "proxy" is
     *             specified, "http_proxy" system environmental variable will
     *             be used.
     */
    void SetProtocol(const std::string &protocol);

    /** Retrieve the disc info from specified database record
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    SDbrBase Pointer to newly created database record object. Caller is
     *             responsible for deleting the object.
     */
    virtual SDbrBase* Retrieve(const int recnum=0) const;

    /** Print the retrieved disc record. If discnum is specified (i.e., valid record ID),
     *  it displays the disc info with tracks. If discnum is omitted or negative, it lists
     *  records with their discid, genre, artist and title.
     *
     *  @param[in] Disc record ID (0-based index to discs). If negative, retrieves info
     *             for all records.
     */
    void Print(const int discnum=-1) const;

private:
    /** Initialize a new disc and fill it with disc info
     *  from the supplied cuesheet and length. Previously created disc
     *  data are discarded. After disc and its tracks are initialized,
     *  CDDB disc ID is computed. If the computation fails, function
     *  throws an runtime_error.
     *
     *  @param[in] CD-ROM device path
     */
    void InitDisc_(const std::string &dev);

    /** Form an artist credit string
     *
     *  @param[in] ArtistCredit query data
     *  @param[in] true to use SortName (if available) for the first asrtist's name
     *             (default is false)
     *  @return    Artist name in plain string
     */
    std::string GetArtistString_(const MusicBrainz5::CArtistCredit &credit, const bool sortfirst=false) const;

    std::string discid;
    MusicBrainz5::CQuery MB5;
    CoverArtArchive::CCoverArt CAA;
    std::deque<MusicBrainz5::CRelease> Releases;
    std::deque<CoverArtArchive::CReleaseInfo> CoverArts;

    int CoverArtSize; // 0-full, 1-large thumbnail (500px), 2-small thumbnail (250px)
};
