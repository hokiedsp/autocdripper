#pragma once

#include <vector>
#include <functional>

#include "enums.h"

struct SCueSheet;
class CDbMusicBrainz;

class IDatabase;
typedef std::vector<IDatabase*> IDatabasePtrVector;
typedef std::vector<std::reference_wrapper<IDatabase>> IDatabaseRefVector;

class IDatabase
{
public:
    virtual ~IDatabase() {}

    /**
     * @brief Return database type enum
     * @return ReleaseDatabase enumuration value
     */
    virtual DatabaseType GetDatabaseType() const=0;

    /**
     * @brief Return true if Database can be searched for a CD info
     * @return true if database can be searched for a CD release
     */
    virtual bool IsReleaseDb() const=0;

    /**
     * @brief Return true if Database can be searched for a CD cover art
     * @return true if database can be searched for a CD cover art
     */
    virtual bool IsImageDb() const=0;

    /**
     * @brief Return true if database can be queried directly from CD info
     * @return true if database can receive CD info based query
     */
    virtual bool AllowQueryCD() const=0;

    /**
     * @brief Return true if MusicBrainz database is known to contain
     *        a link to this database
     * @return true if release ID is obtainable from MusicBrainz
     */
    virtual bool MayBeLinkedFromMusicBrainz() const=0;

    /**
     * @brief Return true if database supports UPC barcode search
     * @return true if database supports UPC barcode search
     */
    virtual bool AllowSearchByUPC() const=0;

    /**
     * @brief Return true if database supports search by album artist and title
     * @return true if database supports search by album artist and title
     */
    virtual bool AllowSearchByArtistTitle() const=0;

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
    virtual int Query(CDbMusicBrainz &mbdb, const std::string upc="")=0;

    /** If AllowSearchByArtistTitle() returns true, Search() performs a new album search based on
     *  album title and artist. If search is not supported or did not return any match,
     *  Search() returns zero.
     *
     *  @param[in] Album title
     *  @param[in] Album artist
     *  @param[in] true to narrowdown existing records; false for new search
     *  @return    Number of matched records
     */
      virtual int Search(const std::string &title, const std::string &artist,bool narrowdown=false)=0;

    /** If AllowSearchByUPC() returns true, Search() performs a new album search based on
     *  album title and artist. If search is not supported or did not return any match,
     *  Search() returns zero.
     *
     *  @param[in] UPC string
     *  @param[in] true to narrowdown existing records; false for new search
     *  @return    Number of matched records
     */
      virtual int Search(const std::string &upc, bool narrowdown=false)=0;

    /**
     * @brief Clear all the matches from previous search
     */
    virtual void Clear()=0;

    ///////////////////////////////////////////////////////////////////////////

    /** Return a unique disc ID if AllowQueryCD()=true
     *
     *  @return discid string
     */
    virtual std::string GetDiscId() const=0;

    /** Returns the number of matched records returned from the last Query() call.
     *
     *  @return    Number of matched records
     */
    virtual int NumberOfMatches() const=0;

    /** Get album UPC
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    UPC string (empty if UPC not available)
     */
    virtual std::string AlbumUPC(const int recnum=0) const=0;

};
