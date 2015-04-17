#pragma once

#include <vector>
#include <string>

#include <libxml/tree.h>

#include "IDatabase.h"
#include "IReleaseDatabase.h"
#include "IImageDatabase.h"
#include "CUtilUrl.h"

class CDbMusicBrainzElem;
class CDbMusicBrainzElemCAA;
class CUtilXmlTree;
class CDbAmazon;

/** Class to access MusicBrainz online CD and coverart databases service.
 */
class CDbMusicBrainz : public IDatabase, public IReleaseDatabase, public IImageDatabase, public CUtilUrl
{
public:
    /** Constructor.
     *
     *  @param[in] Client program name. If omitted or empty, uses "autorip"
     *  @param[in] Client program version. If omitted or empty, uses "alpha"
     */
    CDbMusicBrainz(const std::string &cname="autorip",const std::string &cversion="alpha");

    /** Destructor
     */
    virtual ~CDbMusicBrainz();

    /////////////////////////////////////////
    // for IDatabase

    /**
     * @brief Return true if Database can be searched for a CD info
     * @return true if database can be searched for a CD release
     */
    bool IsReleaseDb() const { return true; }

    /**
     * @brief Return true if Database can be searched for a CD cover art
     * @return true if database can be searched for a CD cover art
     */
    bool IsImageDb() const { return true; }

    /**
     * @brief Return database type enum
     * @return ReleaseDatabase enumuration value
     */
    virtual DatabaseType GetDatabaseType() const { return DatabaseType::MUSICBRAINZ; }

    /**
     * @brief Return true if database can be queried directly from CD info
     * @return true if database can receive CD info based query
     */
    virtual bool AllowQueryCD() const { return true; }

    /**
     * @brief Return true if MusicBrainz database is known to contain
     *        a link to this database
     * @return true if release ID is obtainable from MusicBrainz
     */
    virtual bool MayBeLinkedFromMusicBrainz() const { return false; }

    /**
     * @brief Return true if database supports UPC barcode search
     * @return true if database supports UPC barcode search
     */
    virtual bool AllowSearchByUPC() const { return false; }

    /**
     * @brief Return true if database supports search by album artist and title
     * @return true if database supports search by album artist and title
     */
    virtual bool AllowSearchByArtistTitle() const { return false; }

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
    virtual int Query(const std::string &dev, const SCueSheet &cuesheet,
                      const size_t len, const std::string cdrom_upc="");

    /** If MayBeLinkedFromMusicBrainz() returns true, Query() performs a new
     *  query based on the MusicBrainz query results.
     *
     *  @param[in] MusicBrainz database object.
     *  @param[in] (Optional) UPC barcode
     *  @return    Number of matched records
     */
    virtual int Query(CDbMusicBrainz &mbdb, const std::string upc="") { return 0; }

    /** If AllowSearchByArtistTitle() returns true, Search() performs a new album search based on
     *  album title and artist. If search is not supported or did not return any match,
     *  Search() returns zero.
     *
     *  @param[in] Album title
     *  @param[in] Album artist
     *  @param[in] true to narrowdown existing records; false for new search
     *  @return    Number of matched records
     */
      virtual int Search(const std::string &title, const std::string &artist, bool narrowdown=false) { return 0; }

    /** If AllowSearchByUPC() returns true, Search() performs a new album search based on
     *  album title and artist. If search is not supported or did not return any match,
     *  Search() returns zero.
     *
     *  @param[in] UPC string
     *  @param[in] true to narrowdown existing records; false for new search
     *  @return    Number of matched records
     */
      virtual int Search(const std::string &upc, bool narrowdown=false) { return 0; }

    /**
     * @brief Clear all the matches from previous search
     */
    virtual void Clear();

    ///////////////////////////////////////////////////////////////////////////

    /** Return the discid string
   *
   *  @return discid string.
   */
    virtual std::string GetDiscId() const;

    /** Returns the number of matched records returned from the last Query() call.
   *
   *  @return    Number of matched records
   */
    virtual int NumberOfMatches() const;

    /////////////////////////////////////////
    // for IReleaseDatabase

    /** Return a unique release ID string
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return id string if Query was successful.
     */
    virtual std::string ReleaseId(const int recnum=0) const;

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
    virtual std::string AlbumComposer(const int recnum=0) const;

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

    /** Get catalog number
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Catalog Number string (empty if label not available)
     */
    virtual std::string AlbumCatNo(const int recnum=0) const;

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

    /** Get Amazon Standard Identification Number
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    ASIN string (empty if ASIN not available)
     */
    std::string AlbumASIN(const int recnum=0) const;

    /** Get number of tracks
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    number of tracks
     *  @throw     runtime_error if CD record id is invalid
     */
    virtual int NumberOfTracks(const int recnum=0) const;

    /** Get track title
     *
     *  @param[in] Track number (1-99)
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Title string (empty if title not available)
     *  @throw     runtime_error if track number is invalid
     */
    virtual std::string TrackTitle(int tracknum, const int recnum=0) const;

    /** Get track artist
     *
     *  @param[in] Track number (1-99)
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Artist string (empty if artist not available)
     *  @throw     runtime_error if track number is invalid
     */
    virtual std::string TrackArtist(int tracknum, const int recnum=0) const;

    /** Get track composer
     *
     *  @param[in] Track number (1-99)
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Composer string (empty if artist not available)
     *  @throw     runtime_error if track number is invalid
     */
    virtual std::string TrackComposer(int tracknum, const int recnum=0) const;

    /** Get track ISRC
     *
     *  @param[in] Track number (1-99)
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    ISRC string
     *  @throw     runtime_error if track number is invalid
     */
    virtual std::string TrackISRC(int tracknum, const int recnum=0) const;

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
    virtual UByteVector FrontData(const int recnum=0);

    /** Check if the query returned a front cover
     *
     *  @param[out] image data buffer.
     *  @param[in]  record index (default=0)
     */
    virtual UByteVector BackData(const int recnum=0);

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

    //////////////////////////////////////////
    // Member functions independent of interfaces

    /**
     * @brief Get a related URL.
     * @param[in]  Relationship type (e.g., "discogs", "amazon asin")
     * @param[in]  Record index (default=0)
     * @return URL string or empty if requestd URL type not in the URL
     */
    virtual std::string RelationUrl(const std::string &type, const int recnum=0);

    /** Get a vector of track lengths
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Vector of track lengths in seconds
     *  @throw     runtime_error if release number is invalid
     */
    virtual std::vector<int> TrackLengths(const int recnum=0) const;

    /** Set a server connection protocol.
     *
     *  @param[in] Connection protocol ("cddbp","http", or "proxy"). If omitted
     *             or empty, 'cddbp' is used by the default. If "proxy" is
     *             specified, "http_proxy" system environmental variable will
     *             be used.
     */
    void SetProtocol(const std::string &protocol);

    void SetGrabCoverArtFromAmazon(const bool ena);

private:
    static const std::string base_url;
    std::vector<CDbMusicBrainzElem> Releases;
    std::vector<CDbMusicBrainzElemCAA> CoverArts;

    CDbAmazon *amazon;

    int CoverArtSize; // 0-full, 1-large thumbnail (500px), 2-small thumbnail (250px)

    /** Initialize a new disc and fill it with disc info
     *  from the supplied cuesheet and length. Previously created disc
     *  data are discarded. After disc and its tracks are initialized,
     *  CDDB disc ID is computed. If the computation fails, function
     *  throws an runtime_error.
     *
     *  @param[in] cuesheet representing the disc
     */
    CUtilXmlTree GetNewDiscData_(const SCueSheet &cuesheet);

    /**
     * @brief Check medium-list in a discid release to identify the disc if multi-disc set
     * @param[in] pointer to an XML node for a release
     * @param[in] total time of the CD in sectors
     * @return disc number
     */
    int DiscID_(const xmlNode *release_node, const int trackcount, const size_t totaltime);

    /** Retrieve the front cover data.
     *
     *  @param[out] image data buffer.
     *  @param[in]  record index (default=0)
     */
    bool GetCAA_(const int recnum, CDbMusicBrainzElemCAA &coverart) const;
};
