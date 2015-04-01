#pragma once

#include <string>
#include <vector>
#include <curl/curl.h>

#include "IDatabase.h"
#include "IReleaseDatabase.h"
#include "IImageDatabase.h"
#include "CDbHttpBase.h"
#include "SDbrBase.h"
#include "CUtilJson.h"

class CDbMusicBrainz;
class CDbDiscogsElem;
typedef std::vector<CDbDiscogsElem> CDiscogsElemVector;

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
        public CDbHttpBase
{
public:
    /** Constructor.
     *
     *  @param[in] Client program name. If omitted or empty, uses "autorip"
     *  @param[in] Client program version. If omitted or empty, uses "alpha"
     */
    CDbDiscogs(const std::string &cname="autorip",const std::string &cversion="alpha");

    /** Destructor
     */
    virtual ~CDbDiscogs();

    /**
     * @brief Return true if Database can be searched for a CD info
     * @return true if database can be searched for a CD release
     */
    virtual bool IsReleaseDb() const { return true; }

    /**
     * @brief Return true if Database can be searched for a CD cover art
     * @return true if database can be searched for a CD cover art
     */
    virtual bool IsImageDb() const { return true; }

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

    /**
     * @brief Return database type enum
     * @return ReleaseDatabase enumuration value
     */
    virtual DatabaseType GetDatabaseType() const { return DatabaseType::DISCOGS  ; }

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
                      const size_t len, const std::string cdrom_upc="")
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

    /** Look up full disc information from CDDB server. It supports single record or
      *  multiple records if multiple entries were found by Query(). If the computation
      *  fails, function throws an runtime_error.
      *
      *  @param[in] Disc record ID (0-based index to discs). If negative, retrieves info
      *             for all records.
      *  @param[in] Network time out in seconds. If omitted or negative, previous value
      *             will be reused. System default is 10.
      */
    virtual void Populate(const int recnum=-1) {}

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

    /** Get album UPC
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    UPC string (empty if title not available)
     */
    virtual std::string AlbumUPC(const int recnum=-1) const;

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
    virtual std::string AlbumTitle(const int recnum=-1) const;

    /** Get album artist
         *
         *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
         *             is returned.
         *  @return    Artist string (empty if artist not available)
         */
    virtual std::string AlbumArtist(const int recnum=-1) const;

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
     *  @return    Empty string (Discogs does not support genre)
     */
    virtual std::string Genre(const int recnum=0) const;

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

    // -----------------------------------------------------------------------------

    /** Specify the preferred coverart image width
     *
     *  @param[in] Preferred width of the image
     */
    virtual void SetPreferredWidth(const size_t &width) {}

    /** Specify the preferred coverart image height
     *
     *  @param[in] Preferred height of the image
     */
    virtual void SetPreferredHeight(const size_t &height) {}

    /** Check if the query returned a front cover
     *
     *  @param[in]  record index (default=0)
     *  @return     true if front cover is found.
     */
    virtual bool Front(const int recnum=0) const { return false; }

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
    virtual UByteVector FrontData(const int recnum=0) const { UByteVector rval; return rval; }

    /** Check if the query returned a front cover
     *
     *  @param[out] image data buffer.
     *  @param[in]  record index (default=0)
     */
    virtual UByteVector BackData(const int recnum=0) const { UByteVector rval; return rval; }

    /** Get the URL of the front cover image
     *
     *  @param[in]  Record index (default=0)
     *  @return     URL string
     */
    virtual std::string FrontURL(const int recnum=0) const { return ""; }

    /** Get the URL of the back cover image
     *
     *  @param[in]  Record index (default=0)
     *  @return     URL string
     */
    virtual std::string BackURL(const int recnum=0) const { return ""; }

    // -----------------------------------------------------------

    /**
     * @brief Set Country Preference
     * @param[in] country_code
     */
    void SetCountryPreference(const std::string country_code) { preferred_country = country_code; }

private:
    static const std::string base_url;
    std::string preferred_country;

    CDiscogsElemVector Releases;

    /** Goes through OAuth authorization procedure. Will fork to show user authorization webpage.
     *  Authorize_() will wait for the generated token to be copied back.
     * @brief
     */
    void Authorize_();

    /**
     * @brief Query helper function to analyze master release record
     * @param[in] receives a url to a master release record and returns a url to its oldest CD release
     *            record.
     * @param[in] UPC barcode. Empty if not given
     * @param[in] ID of the last master record queried (0 if none prior).
     * @return ID of the queried master record
     */
    int QueryMaster_(std::string &url, const std::string upc, const int last_id);

    /**
     * @brief Select a release from releases listed under a master release
     *
     * The selection is based on the UPC given in the argument and preferred_country member
     * variable (which can be set via SetCountryPreference() member function). The calling function
     * (QueryMaster_()) is expected to create a new empty entry in Releases member variable for
     * this function to fill. If this function returns true, Releases.back() contains the selected
     * record data.
     *
     * @param[in] JSON object containing versions: /masters/{master_id}/versions
     * @param[in] Optional UPC or empty string if none given
     * @return true if a release is successfully retrieved
     */
    bool SelectFromMasterVersions_(const CUtilJson &versions, const std::string &upc);
};
