#include "CCueSheetBuilder.h"

#include <stdexcept>
#include <algorithm>

#include "CDbMusicBrainz.h"

#include <iostream>
using std::cout;

struct DatabaseElem
{
    IDatabase& eg;
    int recid;

    DatabaseElem(IDatabase& db) : eg(db), recid(-1) {}
};


/**
 * @brief CCueSheetBuilder constructor.
 */
CCueSheetBuilder::CCueSheetBuilder()
    : cdrom_len(0), upc_match(false), db_marge_method(1), canceled(false), matched(false)
{}

/**
 * @brief CCueSheetBuilder destructor.
 */
CCueSheetBuilder::~CCueSheetBuilder() {}

// //////////////////////////////////////////////////////////////////////////////////////
// INPUT related functions

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
 * @brief Add an Album REM field
 * @param[in] REM field to be included
 */
void CCueSheetBuilder::AddRemField(const AlbumRemFieldType field)
{
    // check for duplicate
    bool isnew = true;
    AlbumRemFieldVector::iterator it;
    for (it=remfields.begin(); isnew && it!=remfields.end(); it++)
        isnew = (*it)!=field;

    // if new element, add to the list
    if (isnew) remfields.push_back(field);
}

/**
 * @brief Add REM fields
 * @param[in] a vector of REM fields to be included
 */
void CCueSheetBuilder::AddRemFields(const AlbumRemFieldVector &fields)
{
    AlbumRemFieldVector::const_iterator it;
    for (it=fields.begin(); it!=remfields.end(); it++)
        AddRemField((*it));
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
void CCueSheetBuilder::AddDatabases(const IDatabaseRefVector &dbs)
{
    if (Running()) throw(std::runtime_error("CCueSheetBuilder thread is already running."));

    for (IDatabaseRefVector::const_iterator it=dbs.begin();it!=dbs.end();it++)
        databases.emplace_back(*it);
}

/**
 * @brief Allow combinig of results from multiple databases
 * @param[in] true to allow combining, false to pick a result
 *            from most preferred database
 * @param[in] true to allow combining of database results with common UPC.
 */
void CCueSheetBuilder::AllowCombinig(const bool do_combine, const bool upc_bound)
{
    if (!do_combine) db_marge_method = 0;
    else if (upc_bound) db_marge_method = 2;
    else db_marge_method = 1;
}

/**
 * @brief Set UPC-Matching Requirement
 * @param[in] true to require UPC match (if UPC is given)
 */
void CCueSheetBuilder::RequireUpcMatch(const bool reqmatch)
{
    upc_match = reqmatch;
}

// //////////////////////////////////////////////////////////////////////////////////////
// OUTPUT related functions

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
bool CCueSheetBuilder::FoundBackCover() const
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

// //////////////////////////////////////////////////////////////////////////////////////
// thread function & its helpers

/**
 * @brief Thread function to populate CueSheet given
 */
void CCueSheetBuilder::ThreadMain()
{
    cout << "[CCueSheetBuilder thread] started\n";

    std::string upc(cdrom_upc);

    std::vector<DatabaseElem>::iterator it;
    CDbMusicBrainz *mbdb = NULL;

    canceled = false;
    matched = false;

    // Step 1: Query based on CD info alone
    cout << "[CCueSheetBuilder thread] step 1\n";
    for (it=databases.begin(); it!=databases.end(); it++)
    {
        if (stop_request) goto cancel;

        IDatabase &db = (*it).eg;

        if (db.AllowQueryCD())  // if queryable, query
        {
            // run query
            db.Query(cuesheet, cdrom_upc);
            if (matched) matched = db.NumberOfMatches();

            cout << "[CCueSheetBuilder thread] Found " << db.NumberOfMatches()
                 << " matches in " << to_string(db.GetDatabaseType()) << "\n";

            // if musicbrainz database, save the pointer to it
            if (db.GetDatabaseType() == DatabaseType::MUSICBRAINZ)
                mbdb = &static_cast<CDbMusicBrainz&>(db);

        }
        else // if not queryable, check if it can use MusicBrainz
        {
            // clear the previous match
            db.Clear();
        }
    }

    // ----------------------------------------------------------------------------

    // Step 2: Query based off of MusicBrainz search if possible
    cout << "[CCueSheetBuilder thread] step 2\n";
    if (mbdb) // MusicBrainz DB is included
    {
        for (it=databases.begin(); it!=databases.end(); it++)
        {
            if (stop_request) goto cancel;

            IDatabase &db = (*it).eg;

            // if previous query not succss & queriable off MBDB, query
            if (!db.NumberOfMatches() && db.MayBeLinkedFromMusicBrainz())
            {
                db.Query(*mbdb, cdrom_upc);
                if (db.NumberOfMatches()) matched = true;
            }
        }
    }

    // ----------------------------------------------------------------------------

    // Step 3: Check UPC match & Search UPC if no match
    cout << "[CCueSheetBuilder thread] step 3\n";

    it = databases.begin();

    // If UPC match is not required and UPC is not given, look for the first UPC in DBs
    if (matched && !upc_match && upc.empty())
    {
        for (; it!=databases.end() && upc.empty() ; it++)
        {
            if (stop_request) goto cancel;

            IDatabase &db = (*it).eg;

            // Find the first returned UPC
            for (int rid=0; rid<db.NumberOfMatches() && upc.empty(); rid++)
            {
                std::string this_upc = db.AlbumUPC(rid); // returns empty if not available
                if (this_upc.size())
                {
                    upc = this_upc;
                    (*it).recid = rid;
                }
            }
        }
    }

    // Now search through the DBs (all if upc_match, else continue from prev loop)
    // for the matching UPC if UPC found
    if (upc.size())
    {
        for (; it!=databases.end(); it++)
        {
            if (stop_request) goto cancel;

            IDatabase &db = (*it).eg;
            int &recid = (*it).recid;

            // Look for matching UPC
            for (int rid=0; rid<db.NumberOfMatches(); rid++)
            {
                std::string this_upc = db.AlbumUPC(rid); // returns empty if not available
                if (upc.compare(this_upc)==0) recid = rid;
            }

            // If none exists and DB spports UPC search, search now
            if (recid<0 && db.AllowSearchByUPC() && db.Search(upc))
                recid = 0; // pick the first match
        }
    }

    // ----------------------------------------------------------------------------

    // Step 4: initialize cuesheet's REM fields
    cout << "[CCueSheetBuilder thread] step 4 - Initializing REM fields \n";
    for (size_t i=0; i<remfields.size(); i++) cuesheet.Rems.emplace_back("");

    // Step 5: Populate the cuesheet
    {
        cout << "[CCueSheetBuilder thread] step 5 - Populating cuesheet\n";

        // temporarily use matched flag to control cuesheet popuilation scheme
        matched = false;

        // first with the UPC-matched databases (only if UPC given)
        if (cdrom_upc.size())
        {
            for (it=databases.begin();
                 it!=databases.end() && (!matched || db_marge_method>0);
                 it++)
            {
                if (stop_request) goto cancel;

                int recid = (*it).recid;

                // if contains UPC-matched result, retrieve the match
                if (recid>=0)
                {
                    ProcessDatabase_((*it).eg, recid);
                    matched = true;
                }
            }
        }

        // then look through UPC-unmatched outcomes if allowed
        bool any_recid = upc.empty() || db_marge_method!=2;
        if ((!matched || db_marge_method>0) && (!upc_match || cdrom_upc.empty()))
        {
            cout << "Look through UPC-unmatched outcomes\n";
            for (it=databases.begin();
                 it!=databases.end() && (!matched || db_marge_method>0);
                 it++)
            {
                if (stop_request) goto cancel;

                IDatabase &db = (*it).eg;
                int recid = (*it).recid;

                // if contains a UPC-unmatched result, marge the data to the cuesheet
                if (db.NumberOfMatches() && (any_recid || recid>=0))
                {
                    ProcessDatabase_(db,(recid<0)?0:recid);
                    matched = true;
                }
            }
        }
    }

    // Step 6: Removed unpopulated REMs
    cout << "[CCueSheetBuilder thread] step 6 - Removing unused REM fields\n";
    cuesheet.Rems.erase(std::remove_if(cuesheet.Rems.begin(),
                                       cuesheet.Rems.end(),
                                       [](const std::string& s) { return s.empty(); }),
                        cuesheet.Rems.end());

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
 * @param[in] source database with at least one match
 * @param[in] record index of the matched
 * @return true if database contains a match and db_marge_method==0
 */
void CCueSheetBuilder::ProcessDatabase_(IDatabase &db, const int recid)
{
    if (db.IsReleaseDb()) // marge the data to the cuesheet
    {
        const IReleaseDatabase& rdb = dynamic_cast<const IReleaseDatabase&>(db);
        // get primary album data
        if (cuesheet.Performer.empty() && cuesheet.Songwriter.empty())
        {
            cuesheet.Performer = rdb.AlbumArtist(recid);
            cuesheet.Songwriter = rdb.AlbumComposer(recid);
        }

        if (cuesheet.Title.empty())
            cuesheet.Title = rdb.AlbumTitle(recid);

        // get track data
        for (size_t i=0; i<cuesheet.Tracks.size();)
        {
            SCueTrack &track = cuesheet.Tracks[i++];

            if (track.Performer.empty() && track.Songwriter.empty())
            {
                track.Performer = rdb.TrackArtist(i, recid);
                track.Songwriter = rdb.TrackComposer(i, recid);
            }

            if (track.Title.empty())
                track.Title = rdb.TrackTitle(i, recid);
        }

        // get additional album data
        for (size_t i=0; i<remfields.size();i++)
        {
            std::string &rem = cuesheet.Rems[i];
            if (rem.empty())
            {
                switch (remfields[i])
                {
                case AlbumRemFieldType::DBINFO:
                    rem = "DBINFO " + to_string(rdb.GetDatabaseType()) + " " + rdb.ReleaseId(recid);
                    break;
                case AlbumRemFieldType::GENRE:
                    rem = rdb.Genre(recid);
                    if (rem.size()) rem.insert(0,"GENRE ");
                    break;
                case AlbumRemFieldType::DATE:
                    rem = rdb.Date(recid);
                    if (rem.size()) rem.insert(0,"DATE ");
                    break;
                case AlbumRemFieldType::COUNTRY:
                    rem = rdb.Country(recid);
                    if (rem.size()) rem.insert(0,"COUNTRY ");
                    break;
                case AlbumRemFieldType::UPC:
                    rem = rdb.AlbumUPC(recid);
                    if (rem.size()) rem.insert(0,"UPC ");
                    break;
                case AlbumRemFieldType::LABEL:
                    rem = rdb.AlbumLabel(recid);
                    if (rem.size()) rem.insert(0,"LABEL ");
                    break;
                case AlbumRemFieldType::CATNO:
                    rem = rdb.AlbumCatNo(recid);
                    if (rem.size()) rem.insert(0,"CATNO ");
                    break;
                case AlbumRemFieldType::DISC:
                    if (rdb.TotalDiscs()>1)
                    {
                        int no = rdb.DiscNumber();
                        if (no>0)
                        {
                            rem = std::to_string(no);
                            if (rem.size()) rem.insert(0,"DISC ");
                        }
                    }
                    break;
                case AlbumRemFieldType::DISCS:
                    int no = rdb.TotalDiscs();
                    if (no>1)
                    {
                        rem = std::to_string(no);
                        if (rem.size()) rem.insert(0,"DISCS ");
                    }
                    break;
                }
            }
        }
    }

    if (db.IsImageDb()) // grab the cover image if available
    {
        IImageDatabase &idb = dynamic_cast<IImageDatabase&>(db);
        if (front.empty() && idb.Front()) front = idb.FrontData(0);
        if (back.empty() && idb.Back()) back = idb.BackData(0);
    }
}
