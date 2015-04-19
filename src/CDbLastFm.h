#pragma once

#include <string>
#include <vector>

#include "IDatabase.h"
#include "IReleaseDatabase.h"
#include "IImageDatabase.h"
#include "CUtilUrl.h"

struct SCueSheet;
class CDbMusicBrainz;
class CDbLastFmElem;

typedef std::vector<CDbLastFmElem> CDbLastFmElemVector;

/** Class to access last.fm online CD databases service.
 *
 *  Because of its release information content is inferior to MusicBrainz, this DB is utilized
 *  as an image DB only. Also, this DB is only queried off MusicBrainz ID; hence, it is guaranteed
 *  to be a single result or none.
 */
class CDbLastFm :
        public IDatabase, public IImageDatabase, public CUtilUrl
{
public:
    /** Constructor.
     *
     *  @param[in] Client program last.fm API key
     *  @param[in] Client program name. If omitted or empty, uses "autorip"
     *  @param[in] Client program version. If omitted or empty, uses "alpha"
     */
    CDbLastFm(const std::string &apikey, const std::string &cname="autorip", const std::string &cversion="alpha");

    /** Destructor
     */
    virtual ~CDbLastFm();

    /**
     * @brief Return database type enum
     * @return ReleaseDatabase enumuration value
     */
    virtual DatabaseType GetDatabaseType() const { return DatabaseType::LASTFM; }

    /**
     * @brief Return true if Database can be searched for a CD info
     * @return true if database can be searched for a CD release
     */
    bool IsReleaseDb() const { return false; }

    /**
     * @brief Return true if Database can be searched for a CD cover art
     * @return true if database can be searched for a CD cover art
     */
    bool IsImageDb() const { return true; }

    /**
     * @brief Return true if database can be queried directly from CD info
     * @return true if database can receive CD info based query
     */
    virtual bool AllowQueryCD() const { return false; }

    /**
     * @brief Return true if MusicBrainz database is known to contain
     *        a link to this database
     * @return true if release ID is obtainable from MusicBrainz
     */
    virtual bool MayBeLinkedFromMusicBrainz() const { return true; }

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
     *  @param[in] Cuesheet with its basic data populated
     *  @param[in] (Optional) UPC barcode
     *  @return    Number of matched records
     */
    virtual int Query(const SCueSheet &cuesheet, const std::string cdrom_upc="")
    { return 0; }

    /** If MayBeLinkedFromMusicBrainz() returns true, Query() performs a new
     *  query based on the MusicBrainz query results.
     *
     *  @param[in] MusicBrainz database object.
     *  @param[in] (Optional) UPC barcode
     *  @return    Number of matched records
     */
    virtual int Query(CDbMusicBrainz &mbdb, const std::string upc="");

    /** If AllowSearchByArtistTitle() returns true, Search() performs a new album search based on
     *  album title and artist. If search is not supported or did not return any match,
     *  Search() returns zero.
     *
     *  @param[in] Album title
     *  @param[in] Album artist
     *  @param[in] true to narrowdown existing records; false for new search
     *  @return    Number of matched records
     */
    virtual int Search(const std::string &title, const std::string &artist,
                       bool narrowdown=false) { return 0; }

    /** If AllowSearchByUPC() returns true, Search() performs a new album search based on
     *  album title and artist. If search is not supported or did not return any match,
     *  Search() returns zero.
     *
     *  @param[in] UPC string
     *  @param[in] true to narrowdown existing records; false for new search
     *  @return    Number of matched records
     */
    virtual int Search(const std::string &upc, bool narrowdown=false)
    { return 0; }

    /**
     * @brief Clear all the matches from previous search
     */
    virtual void Clear();

    // -----------------------------------------------------------------------

    /** Return a unique disc ID if AllowQueryCD()=true
     *
     *  @return discid string
     */
    virtual std::string GetDiscId() const { return ""; }

    /** Returns the number of matched records returned from the last Query() call.
     *
     *  @return    Number of matched records
     */
    virtual int NumberOfMatches() const;

    /** Return the ID of the release
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return ID string
     */
    virtual std::string ReleaseId(const int recnum=-1) const;

    /** Get album UPC
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    UPC string (empty if title not available)
     */
    virtual std::string AlbumUPC(const int recnum=-1) const { return ""; }

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
    virtual UByteVector FrontData(const int recnum=0);

    /** Check if the query returned a front cover
     *
     *  @param[out] image data buffer.
     *  @param[in]  record index (default=0)
     */
    virtual UByteVector BackData(const int recnum=0) { UByteVector data; return data; }

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

    CDbLastFmElemVector Releases;
};
