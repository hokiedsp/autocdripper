#pragma once

#include <deque>
#include <string>
#include <cddb/cddb.h>

#include "SCueSheet.h"
#include "SDbrBase.h"

/** CD Database Record structure - SCueSheet with DbType()
 */
struct SDbrCDDB : SDbrBase
{
    /** Name of the database the record was retrieved from
     *
     *  @return Name of the database
     */
    virtual std::string SourceDatabase() const { return "CDDB"; }

    friend std::ostream& operator<<(std::ostream& stdout, const SDbrCDDB& obj);
};

/** Overloaded stream insertion operator to output the content of SDbrCDDB
 *  object. The output is in accordance with the CDRWIN's Cue-Sheet syntax
 *
 *  @param[in]  Reference to an std::ostream object
 *  @return     Copy of the stream object
 */
inline std::ostream& operator<<(std::ostream& os, const SDbrCDDB& o)
{
    os << dynamic_cast<const SCueSheet&>(o);
    return os;
}

class CDbCDDB: public IDatabase
{
public:
    /** Constructor.
     *
     *  @param[in] CDDB server name. If omitted or empty, default server
     *             "freedb.org" is used.
     *  @param[in] CDDB server port. If omitted or negative, 8080 is used as
    *             the default value.
     *  @param[in] Connection protocol ("cddbp","http", or "proxy"). If omitted
     *             or empty, 'cddbp' is used by the default. If "proxy" is
     *             specified, "http_proxy" system environmental variable will
     *             be used.
     *  @param[in] Set the user name and host name of the local machine. If omitted
     *             or empty, no action is taken.
     *  @param[in] Local cache mode: "on", "off", or "only". Default is "on".
     *  @param[in] Local cache directory. Default is "~/.cddbslave".
     *  @param[in] Client program name. If omitted or empty, no action is taken.
     *  @param[in] Client program version. If omitted or empty, no action is taken.
     */
    CDbCDDB(const std::string &servername=std::string(), const int serverport=-1,
            const std::string &protocol=std::string(),  const std::string &email=std::string(),
            const std::string &cachemode=std::string(), const std::string &cachedir=std::string(),
            const std::string &cname=std::string(),const std::string &cversion=std::string());

    /** Destructor
     */
    virtual ~CDbCDDB();

    /** Set a server connection protocol.
     *
     *  @param[in] Connection protocol ("cddbp","http", or "proxy"). If omitted
     *             or empty, 'cddbp' is used by the default. If "proxy" is
     *             specified, "http_proxy" system environmental variable will
     *             be used.
     */
    void SetProtocol(const std::string &protocol);

    /** Set Local cache settings
     *
     *  @param[in] Local cache mode: "on", "off", or "only". If empty, left unchanged.
     *  @param[in] Local cache directory. If empty, left unchanged.
     */
    void SetCacheSettings(const std::string &cachemode=std::string(), const std::string &cachedir=std::string());

    /** Perform a new CDDB query for the disc info given in the supplied cuesheet
     *  and its length. Previous query outcome discarded. After disc and its tracks are initialized,
     *  CDDB disc ID is computed. If the computation fails, function
     *  throws an runtime_error.
     *
     *  After successful Query() call, NumberOfMatches() and
     *
     *  @param[in] CD-ROM device path (not used)
     *  @param[in] Cuesheet with its basic data populated
     *  @param[in] Length of the CD in sectors
     *  @param[in] If true, immediately calls Read() to populate disc records.
     *  @param[in] Network time out in seconds. If omitted or negative, previous value
     *             will be reused. System default is 10.
     *  @return    Number of matched records
     */
    virtual int Query(const std::string &dev, const SCueSheet &cuesheet, const size_t len,
                      const bool autofill=false, const int timeout=-1);

    /** Return the CDDB discid
     *
     *  @return CDDB discid (8 hexdigits) if Query() has been completed successfully.
     *          Otherwise "00000000".
     */
    virtual std::string GetDiscId() const;

    /** Returns the number of matched records returned from the last Query() call.
     *
     *  @return    Number of matched records
     */
    virtual int NumberOfMatches() const;

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

    /** Get genre
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Genre string (empty if genre not available)
     */
    virtual std::string Genre(const int recnum=0) const;

    /** Get label name
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Label string (empty if label not available)
     */
    virtual std::string AlbumLabel(const int recnum=0) const { return ""; }

    /** Get album UPC
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    UPC string (empty if UPC not available)
     */
    virtual std::string AlbumUPC(const int recnum=0) const { return ""; }

    /** Get album ASIN (Amazon Standard Identification Number)
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    ASIN string (empty if ASIN not available)
     */
    virtual std::string AlbumASIN(const int recnum=0) const { return ""; }

    /** Returns the CD record ID associated with the specified genre. If no matching
     *  record is found, it returns -1.
     *
     *  @return Matching CD record ID.
     */
    virtual int MatchByGenre(const std::string &genre) const;

    /** Look up full disc information from CDDB server. It supports single record or
     *  multiple records if multiple entries were found by Query(). If the computation
     *  fails, function throws an runtime_error.
     *
     *  @param[in] Disc record ID (0-based index to discs). If negative, retrieves info
     *             for all records.
     *  @param[in] Network time out in seconds. If omitted or negative, previous value
     *             will be reused. System default is 10.
     */
    virtual void Populate(const int discnum=-1, const int timeout=-1);

    /** Retrieve the disc info from specified database record
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    SDbrBase Pointer to newly created database record object. Caller is
     *             responsible for deleting the object.
     */
    virtual SDbrBase* Retrieve(const int recnum=0) const;

    /** Print the retrieved disc record. If discnum is specified (i.e., valid record ID),
     *  it displays the disc info with tracks. If discnum is omitted or negative, it lists
     *  records with their discid, genre, artist and title.
     *
     *  @param[in] Disc record ID (0-based index to discs). If negative, retrieves info
     *             for all records.
     */
    void Print(const int discnum=-1) const;

private:
    /** Initialize a new disc and fill it with disc info
     *  from the supplied cuesheet and length. Previously created disc
     *  data are discarded. After disc and its tracks are initialized,
     *  CDDB disc ID is computed. If the computation fails, function
     *  throws an runtime_error.
     *
     *  @param[in] Cuesheet with its basic data populated
     *  @param[in] Length of the CD in sectors
     */
    void InitDisc_(const SCueSheet &cuesheet, const size_t len);

    /** Clear all the disc entries
     */
    void ClearDiscs_();

    cddb_conn_t *conn;   /* libcddb connection structure */
    std::deque<cddb_disc_t*> discs;   /* collection of libcddb disc structure */

    static int num_instances;	// keep up with # of active instances
};

