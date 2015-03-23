#include "CCueSheetBuilder.h"

#include <stdexcept>

#include "CDbMusicBrainz.h"

struct DatabaseElem
{
    IDatabase& eg;
    int recid;

    DatabaseElem(const IDatabase& db) : eg(db), recid(-1) {}
};


/**
 * @brief CCueSheetBuilder constructor.
 */
CCueSheetBuilder::CCueSheetBuilder() : canceled(false), matched(false), cdrom_len(0)
{}

/**
 * @brief CCueSheetBuilder destructor.
 */
CCueSheetBuilder::~CCueSheetBuilder() {}

/**
 * @brief Set CD information prior to Start()
 * @param[in] cdrom object
 * @param[in] UPC barcode string (optional)
 * @throw runtime_error if thread is already running
 */
void CCueSheetBuilder::SetCdInfo(const CSourceCdda &cdrom, std::string upc)
{
    if (Running()) throw(std::runtime_error("CCueSheetBuilder thread is already running."));

    cdrom_path = cdrom.GetDevicePath();
    cdrom_len = cdrom.GetLength();
    cuesheet = cdrom.GetCueSheet();
    cdrom_upc = upc;
}

/**
 * @brief Add database object to the search list
 * @param[in] IDatabase reference
 * @throw runtime_error if thread is already running
 */
void CCueSheetBuilder::AddDatabase(IDatabase &db)
{
    if (Running()) throw(std::runtime_error("CCueSheetBuilder thread is already running."));

    databases.emplace_back(db);
}

/**
 * @brief Add multiple databases to search list
 * @param[in] vector of databases in the order of search
 * @throw runtime_error if thread is already running
 */
void CCueSheetBuilder::AddDatabase(const IDatabaseRefVector &dbs)
{
    if (Running()) throw(std::runtime_error("CCueSheetBuilder thread is already running."));

    for (IDatabaseRefVector::iterator it=dbs.begin();it!=dbs.end();it++)
        databases.emplace_back(*it);
}

/**
 * @brief Returns true if any cuesheet info was found
 * @return true if any cuesheet info was found
 */
bool CCueSheetBuilder::FoundRelease() const
{
    // database match found & process completed
    return ~(Running() || canceled) && matched;
}

/**
 * @brief Get populated cuesheet
 * @return Read-only reference to the internal cuesheet
 */
const SCueSheet &CCueSheetBuilder::GetCueSheet() const
{
    return cuesheet;
}

/**
 * @brief Return true if front cover image was found
 * @return true if front cover image was found
 */
bool CCueSheetBuilder::FoundFrontCover() const
{
    // database match found & process completed
    return ~(Running() || canceled) && front.size();
}

/**
 * @brief Return true if back cover image was found
 * @return true if back cover image was found
 */
bool FoundBackCover() const
{
    // database match found & process completed
    return ~(Running() || canceled) && back.size();
}

/**
 * @brief Get retrieved front cover image data
 * @return Read-only reference to the cover image data
 */
const UByteVector CCueSheetBuilder::GetFrontCover() const
{
    return front;
}

/**
 * @brief Get retrieved back cover image data
 * @return Read-only reference to the cover image data
 */
const UByteVector CCueSheetBuilder::GetBackCover() const
{
    return back;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Thread function to populate CueSheet given
 */
void CCueSheetBuilder::ThreadMain()
{
    bool check_link_in_mb = false;
    std::string upc(cdrom_upc);

    std::vector<DatabaseElem>::iterator it;
    CDbMusicBrainz *mbdb = NULL;

    canceled = false;
    matched = false;

    // Step 1: Query based on CD info alone
    for (it=databases.begin(); it!=databases.end(); it++)
    {
        if (stop_request) goto cancel;

        IDatabase &db = (*it).get().eg;

        if (db.AllowQueryCD())  // if queryable, query
        {
            // if musicbrainz database, save the pointer to it
            if (db.GetDatabaseType() == DatabaseType::MUSICBRAINZ)
                mbdb = &static_cast<CDbMusicBrainz&>(db);

            // run query
            db.Query(cdrom_path, cuesheet, cdrom_len, cdrom_upc);
        }
    }
    else // if not queryable, check if it can use MusicBrainz
    {
        // clear the previous match
        db.Clear();

        // check if MusicBrainz may return a link to it
        check_link_in_mb = check_link_in_mb || db.MayBeLinkedFromMusicBrainz();
    }

    // Step 2: Query based off of MusicBrainz search if possible
    if (mbdb && check_link_in_mb) // MusicBrainz DB is included
    {
        for (it=databases.begin(); it!=databases.end(); it++)
        {
            if (stop_request) goto cancel;

            IDatabase &db = (*it).get().eg;

            // if previous query not succss & queriable off MBDB, query
            if (!db.NumberOfMatches() && db.MayBeLinkedFromMusicBrainz())
                db.Query(*mbdb, cdrom_upc);
        }
    }

    // Step 3: If UPC is not known, use the UPC of the first entry with the UPC
    if (upc.empty())
    {
        for (it=databases.begin(); it!=databases.end() && upc.empty(); it++)
        {
            if (stop_request) goto cancel;

            IDatabase &db = (*it).get().eg;

            if (db.NumberOfMatches() && db.IsReleaseDb())
            {
                IReleaseDatabase &rdb = static_cast<IReleaseDatabase>(db);

                for (int rid=0;rid<db.NumberOfMatches() && upc.empty();rid++)
                {
                    // Retrieve the UPC
                    std::string this_upc = rdb.AlbumUPC(rid); // returns empty if not available
                    if (this_upc.size()) upc = this_upc;
                }
            }
        }
    }

    // Step 4: Go over databases again to find the records with matching UPC
    if (upc.size()) // upc is resolved (either given or from earlier database results)
    {
        for (it=databases.begin(); it!=databases.end() && upc.empty(); it++)
        {
            if (stop_request) goto cancel;

            IDatabase &db = (*it).get().eg;
            int &recid = (*it).get().recid;

            if (db.AllowSearchByUPC)
            {
                if (db.NumberOfMatches()) // already returned results
                {
                    // narrow-down the results if possible
                    if (db.SearchByUPC(upc,true)) recid = 0;
                }
                else // none found previously
                {
                    // new search
                    if (db.SearchByUPC(upc,false)) recid = 0;
                }
            }
            else if (db.NumberOfMatches() && db.IsReleaseDb())
            {
                // if db cannot be searched for UPC, still check if
                // the matching UPC was found

                IReleaseDatabase &rdb = static_cast<IReleaseDatabase>(db);

                for (int rid=0; rid<db.NumberOfMatches() && recid<0; rid++)
                {
                    std::string this_upc = rdb.AlbumUPC(rid); // returns empty if not available
                    if (this_upc.compare(upc)) recid = rid;
                }
            }
        }
    }

    // Step 5: Populate the cuesheet, first with the UPC-matched databases (only if UPC given)
    if (cdrom_upc.size())
    {
        for (it=databases.begin(); it!=databases.end(); it++)
        {
            if (stop_request) goto cancel;

            IDatabase &db = (*it).get().eg;
            int &recid = (*it).get().recid;

            // set matched flag
            matched = matched || db.NumberOfMatches();

            // if contains UPC-matched result...
            if (recid>=0)
            {
                if (db.IsReleaseDb()) // marge the data to the cuesheet
                    BuildCueSheet_(cuesheet,static_cast<IReleaseDatabase>(db),recid);

                if (db.IsImageDb()) // grab the cover image if available
                {
                    IImageDatabase idb = static_cast<IImageDatabase>(db);
                    if (front.empty() && idb.Front()) front = idb.FrontData(recid);
                    if (back.empty() && idb.Back()) back = idb.BackData(recid);
                }
            }
        }
    }

    // Step 6: Further populate the cuesheet with the UPC-less database matches
    for (it=databases.begin(); it!=databases.end(); it++)
    {
        if (stop_request) goto cancel;

        IDatabase &db = (*it).get().eg;
        int &recid = (*it).get().recid;

        // if contains a UPC-unmatched result, marge the data to the cuesheet
        if ((recid<0 || cdrom_upc.empty()) && db.NumberOfMatches())
        {
            if (db.IsReleaseDb()) // marge the data to the cuesheet
                BuildCueSheet_(cuesheet,static_cast<IReleaseDatabase>(db),0);

            if (db.IsImageDb()) // grab the cover image if available
            {
                IImageDatabase idb = static_cast<IImageDatabase>(db);
                if (front.empty() && idb.Front()) front = idb.FrontData(0);
                if (back.empty() && idb.Back()) back = idb.BackData(0);
            }
        }
    }

    // all completed
    return;

    // cancelled by external means
cancel:
    canceled = true;
    return;
}

/**
 * @brief Internal function to be called by ThreadMain to build the
 *        cuesheet from database.
 * @param[inout] cuesheet to accumulate data
 * @param[in] source database
 * @param[in] record index of the matched
 */
void CCueSheetBuilder::BuildCueSheet_(SCueSheet &cuesheet, const IReleaseDatabase &db, const int recid)
{

}
