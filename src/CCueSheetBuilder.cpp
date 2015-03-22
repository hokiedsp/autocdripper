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

            // run query first
            db.Query(cdrom_path, cuesheet, cdrom_len);

            // match found in a release database
            if (db.NumberOfMatches() && db.IsReleaseDb())
            {
                IReleaseDatabase &rdb = static_cast<IReleaseDatabase>(db);
                int &recid = (*it).get().recid;

                for (int rid=0;rid<db.NumberOfMatches();rid++)
                {
                    // retrieve the UPC (empty if not available)
                    std::string this_upc = rdb.AlbumUPC(recnum);

                    if (cdrom_upc.empty()) // actual UPC unknown
                    {
                        if (this_upc.size()) // DB UPC known
                        {
                            if (upc.empty())
                            {
                                upc = this_upc;
                                recid = rid;
                                break;
                            }
                            else if (upc.compare(this_upc)==0)
                            {
                                recid = rid;
                                break;
                            }
                        }
                    }
                    else // actual UPC known
                    {
                         // DB UPC known && matches the actual UPC
                        if (this_upc.size() && cdrom_upc.compare(this_upc)==0)
                        {
                            recid = rid;
                            break;
                        }
                    }
                }
            }
        }
        else // if not queryable, check if it can use MusicBrainz
        {
            // clear the previous match
            db.Clear();

            // check if MusicBrainz may return a link to it
            check_link_in_mb = check_link_in_mb || db.MayBeLinkedFromMusicBrainz();
        }
    }



    // all completed
    return;

    // cancelled by external means
cancel:
    canceled = true;
    return;
}
