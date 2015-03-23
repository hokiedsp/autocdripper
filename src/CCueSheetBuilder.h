#prgma once

#include "CThreadManBase.h"
#include "IDatabase.h"
#include "IReleaseDatabase.h"

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
 * SetCdInfo() - gathers the CD info (device path, track info, and
 *               length) from a CSourceCdda object.
 * AddDatabase() - appends the databases to search. Each database
 *                 object must be configured externally and must
 *                 be ready to query/search before Start() is called.
 *
 *                 The priority of the databases is set by the order
 *                 they are added. If multiple databases were assigned
 *                 at once with IDatabaseRefVector, the lower the index
 *                 in the vector, higher its priorities are.
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

    /**
     * @brief Returns the status of last thread run
     * @return true if its thread was externally stopped prematurely during
     *         its last run.
     */
    bool Canceled() const { return canceled; }

    /**
     * @brief Set CD information prior to Start()
     * @param[in] cdrom object
     * @param[in] UPC barcode string (optional)
     * @throw runtime_error if thread is already running
     */
    void SetCdInfo(const CSourceCdda &cdrom, std::string upc="");

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
    void AddDatabase(const IDatabaseRefVector &dbs);

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
    bool canceled;
    bool matched;

    std::string cdrom_path;
    size_t cdrom_len;
    std::string cdrom_upc;

    std::vector<DatabaseElem> databases;

    SCueSheet cuesheet;
    UByteVector front;
    UByteVector back;

    /**
     * @brief Internal function to be called by ThreadMain to build the
     *        cuesheet from database.
     * @param[inout] cuesheet to accumulate data
     * @param[in] source database
     * @param[in] record index of the matched
     */
    static void BuildCueSheet_(SCueSheet &cuesheet, const IReleaseDatabase &db, const int recid);
};
