#pragma once

#include "CThreadManBase.h"
#include "IDatabase.h"
#include "IReleaseDatabase.h"
#include "IImageDatabase.h"

#include "CSourceCdda.h"
#include "SCueSheet.h"
#include "enums.h"

struct DatabaseElem;

/**
 * @brief The CCueSheetBuilder class
 *
 * CCueSheetBuilder is a thread managing class, of which thread
 * accesses multiple online music databases to gather information
 * about the music CD currently in the optical disc drive. The CD
 * information includes album artist and title, track artists and
 * titles, cover art images, and other miscellaneous info.
 *
 * A CCueSheetBuilder object first needs to be initialized before
 * starting its thread execution by calling the following member
 * functions:
 *
 * SetCdInfo() - Gathers the CD info (device path, track info, and
 *               length) from a CSourceCdda object as well as the user-supplied
 *               UPC.
 *
 * AddDatabase()  - Appends the databases to search. Each database
 * AddDatabases()   object must be configured externally and must
 *                  be ready to query/search before Start() is called.
 *
 *                  The priority of the databases is set by the order
 *                  they are added. If multiple databases were assigned
 *                  at once with IDatabaseRefVector, the lower the index
 *                  in the vector, higher its priorities are.
 *
 * AddRemField()  - Registers album REM fields to be included in the cue
 * AddRemFields()   sheet. The regiestered REM fields appear in the cue
 *                  sheet in the order added.
 *
 * RequireUpcMatch() - If UPC is given, only accept results with the matching UPC
 * AllowCombining()  - Combine results from multiple databases
 *
 * Once CD info and databases are set, call Start() to begin gathering
 * the information. WaitTillDone() maybe called by the calling thread
 * (or any other threads for that matter).
 *
 * If multiple matches are found in a database, only the first match
 * will be considered. The matches across databases are combined in the
 * priority order. Each cuesheet field is filled with the retrieved
 * information from the highest priority database with the field info.
 *
 * Once the thread has completed its task, call
 *
 * FoundRelease() to check whether any match was found in any databases
 * FoundFrontCover() to check whether front cover image was found
 * FoundBackCover() to check whether back cover image was found
 *
 * Then, to access the gathered data, call GetCueSheet(),
 * GetFrontCover(), and GetRearCover().
 *
 */
class CCueSheetBuilder : public CThreadManBase
{
public:
    /**
     * @brief CCueSheetBuilder constructor.
     */
    CCueSheetBuilder();

    /**
     * @brief CCueSheetBuilder destructor.
     */
    virtual ~CCueSheetBuilder();

    //-----------------------------------------------------
    // PRE-THREAD functions

    /**
     * @brief Set CD information prior to Start()
     * @param[in] cdrom object
     * @param[in] UPC barcode string (optional)
     * @throw runtime_error if thread is already running
     */
    void SetCdInfo(const CSourceCdda &cdrom, std::string upc="");

    /**
     * @brief Add an Album REM field
     * @param[in] REM field to be included
     */
    void AddRemField(const AlbumRemFieldType field);

    /**
     * @brief Add REM fields
     * @param[in] a vector of REM fields to be included
     */
    void AddRemFields(const AlbumRemFieldVector &fields);

    /**
     * @brief Add database object to the search list
     * @param[in] IDatabase reference
     * @throw runtime_error if thread is already running
     */
    void AddDatabase(IDatabase &db);

    /**
     * @brief Add multiple databases to search list
     * @param[in] vector of databases in the order of search
     * @throw runtime_error if thread is already running
     */
    void AddDatabases(const IDatabaseRefVector &dbs);

    /**
     * @brief Allow combinig of results from multiple databases
     * @param[in] true to allow combining, false to pick a result
     *            from most preferred database
     * @param[in] true to allow combining of database results with common UPC.
     *
     * Turning off this feature makes CCueSheetBuilder to fill all the cuesheet fields
     * from the first database with a result. Conversely, turning on this feature
     * (which is on by the default) makes CCueSheetBuilder to fill each field of the
     * cuesheet with the first non-empty result from the databases. Setting the second
     * argument to true limits the combining to the databases with matched UPC.
     *
     * This is useful feature when using a combination of MusicBrainz and FreeDb services
     * as MusicBrainz returns more detailed information but lacks genre while FreeDb
     * lacks information but returns a genre.
     *
     * Implementation-wise, this function sets db_marge_method private variable
     */
    void AllowCombinig(const bool do_combine, const bool upc_bound);

    /**
     * @brief Set UPC-Matching Requirement
     * @param[in] true to require UPC match (if UPC is given)
     */
    void RequireUpcMatch(const bool reqmatch);

    //-----------------------------------------------------
    // POST-THREAD functions

    /**
     * @brief Returns the status of last thread run
     * @return true if its thread was externally stopped prematurely during
     *         its last run.
     */
    bool Canceled() const { return canceled; }

    /**
     * @brief Returns true if any cuesheet info was found
     * @return true if any cuesheet info was found
     */
    bool FoundRelease() const;

    /**
     * @brief Get populated cuesheet
     * @return Read-only reference to the internal cuesheet
     */
    const SCueSheet &GetCueSheet() const;

    /**
     * @brief Return true if front cover image was found
     * @return true if front cover image was found
     */
    bool FoundFrontCover() const;

    /**
     * @brief Return true if back cover image was found
     * @return true if back cover image was found
     */
    bool FoundBackCover() const;

    /**
     * @brief Get retrieved front cover image data
     * @return Read-only reference to the cover image data
     */
    const UByteVector GetFrontCover() const;

    /**
     * @brief Get retrieved back cover image data
     * @return Read-only reference to the cover image data
     */
    const UByteVector GetBackCover() const;

protected:
    /**
     * @brief Thread function to populate CueSheet given
     */
    virtual void ThreadMain();

private:
    std::string cdrom_path;
    size_t cdrom_len;
    std::string cdrom_upc;

    std::vector<DatabaseElem> databases;
    AlbumRemFieldVector remfields;   // specifies which REM field to add

    bool upc_match; // true to only accept matched UPC records (if UPC given)
    int db_marge_method; // 0-single db, 1-multiple db, 2-multiple db with UPC constraint

    bool canceled;  // if true after thread is stopped, thread was externally canceled
    bool matched;   // if true after thread is stopped, cuesheet was successfully populated

    SCueSheet cuesheet;
    UByteVector front;
    UByteVector back;

    /**
     * @brief Internal function to be called by ThreadMain to build the
     *        cuesheet from database.
     * @param[in] source database
     * @param[in] record index of the matched
     */
    void ProcessDatabase_(IDatabase &db, const int recid);
};
