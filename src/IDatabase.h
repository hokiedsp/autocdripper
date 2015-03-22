#pragma once

#include <vector>
#include <functional>

#include "enums.h"

struct SCueSheet;

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
    virtual bool IsReleaseDb()=0;

    /**
     * @brief Return true if Database can be searched for a CD cover art
     * @return true if database can be searched for a CD cover art
     */
    virtual bool IsImageDb()=0;

    /**
     * @brief Return true if database can be queried directly from CD info
     * @return true if database can receive CD info based query
     */
    virtual bool AllowQueryCD()=0;

    /**
     * @brief Return true if MusicBrainz database is known to contain
     *        a link to this database
     * @return true if release ID is obtainable from MusicBrainz
     */
    virtual bool MayBeLinkedFromMusicBrainz()=0;

    /**
     * @brief Return true if database supports UPC barcode search
     * @return true if database supports UPC barcode search
     */
    virtual bool AllowSearchByUPC()=0;

    /**
     * @brief Return true if database supports search by album artist and title
     * @return true if database supports search by album artist and title
     */
    virtual bool AllowSearchByArtistTitle()=0;

    /**
     * @brief Return true if database supports search by CDDBID
     * @return true if database supports search by CDDBID
     */
    virtual bool AllowSearchByCDDBID()=0;

    /**
     * @brief Return true if database supports search by MusicBrainz ID
     * @return true if database supports search by MusicBrainz ID
     */
    virtual bool AllowSearchByMBID()=0;

    /** If AllowQueryCD() returns true, Query() performs a new query for the CD info
     *  in the specified drive with its *  tracks specified in the supplied cuesheet
     *  and its length. Previous query outcome discarded. After disc and its tracks
     *  are initialized, CDDB disc ID is computed. If the computation fails, function
     *  throws an runtime_error.
     *
     *  @param[in] CD-ROM device path
     *  @param[in] Cuesheet with its basic data populated
     *  @param[in] Length of the CD in sectors
     *  @param[in] If true, immediately calls Read() to populate disc records.
     *  @param[in] Network time out in seconds. If omitted or negative, previous value
     *             will be reused. System default is 10.
     *  @return    Number of matched records
     */
      virtual int Query(const std::string &dev, const SCueSheet &cuesheet, const size_t len, const bool autofill=false, const int timeout=-1)=0;

    /**
     * @brief Clear all the matches from previous search
     */
    virtual void Clear()=0;

    ///////////////////////////////////////////////////////////////////////////

    /** Return the discid string
   *
   *  @return discid string if Query was successful.
   */
    virtual std::string GetDiscId() const=0;

    /** Returns the number of matched records returned from the last Query() call.
   *
   *  @return    Number of matched records
   */
    virtual int NumberOfMatches() const=0;

};