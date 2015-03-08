#pragma once

#include <string>
#include <curl/curl.h>
#include <jansson.h>

#include "CDbHttpBase.h"
#include "CDbJsonBase.h"
//#include "ICoverArt.h"
#include "SDbrBase.h"

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
class CDbDiscogs : public CDbHttpBase, public CDbJsonBase
{
public:
    /** Constructor.
     *
     *  @param[in] Client program name. If omitted or empty, uses "autorip"
     *  @param[in] Client program version. If omitted or empty, uses "alpha"
     */
    CDbDiscogs(const std::string &cname="autorip",const std::string &cversion="alpha");

    /** Constructor.
     *
     *  @param[in] Client program name. If omitted or empty, uses "autorip"
     *  @param[in] Client program version. If omitted or empty, uses "alpha"
     */
    CDbDiscogs(const CDbMusicBrainz &mb, const std::string &cname="autorip",const std::string &cversion="alpha");

    /** Destructor
     */
    virtual ~CDbDiscogs();

    /** Returns false as Discogs database cannot be queried based on CD track info.
     *
     *  @return    true if query is supported
     */
    static bool IsQueryable() const { return false; }

    /** Returns true as Discogs database can be searched by album title and artist.
     *
     *  @return    true if search is supported
     */
    static bool IsSearchable() const { return true; }

    /** Discogs does not support direct CD query. Always returns 0.
     *
     *  @param[in] CD-ROM device path
     *  @param[in] Cuesheet with its basic data populated (not used)
     *  @param[in] Length of the CD in seconds (not used)
     *  @param[in] If true, immediately calls Read() to populate disc records.
     *  @param[in] Network time out in seconds. (not used)
     *  @return    Number of matched records
     */
    virtual int Query(const std::string &dev, const SCueSheet &cuesheet, const size_t len, const bool autofill=false, const int timeout=-1)
    { return 0; }

    /** Perform a new Discogs query given a list of its release IDs
     *
     *  @param[in] List of release IDs
     *  @return    Number of valid records
     */
    virtual int Query(const std::deque<std::string> &list);

    /** If IsSearchable() returns true, Search() performs a new album search based on
     *  album title and artist. If search is not supported or did not return any match,
     *  Search() returns zero.
     *
     *  @param[in] Album title
     *  @param[in] Album artist
     *  @param[in] If true, immediately calls Read() to populate disc records.
     *  @param[in] Network time out in seconds. If omitted or negative, previous value
     *             will be reused. System default is 10.
     *  @return    Number of matched records
     */
    virtual int Search(const std::string &title, const std::string &artist, const bool autofill=false, const int timeout=-1);

    /** Return the Discogs discid string
     *
     *  @return Discogs discid string if Query was successful. Otherwise "00000000".
     */
    virtual std::string GetDiscId() const { return ""; }

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

    /** Get genre
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Empty string (Discogs does not support genre)
     */
    virtual std::string Genre(const int recnum=0) const { return ""; }

    /** Get label name
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    Label string (empty if label not available)
     */
    virtual std::string AlbumLabel(const int recnum=0) const;

    /** Get album UPC
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    UPC string (empty if title not available)
     */
    virtual std::string AlbumUPC(const int recnum=-1) const;

    /** Returns the CD record ID associated with the specified genre. If no matching
     *  record is found, it returns -1.
     *
     *  @return Matching CD record ID.
     */
    virtual int MatchByGenre(const std::string &genre) const;

    /** No action performed as the full data is retrieved by query/search call.
     *
     *  @param[in] Disc record ID (0-based index to discs). If negative, retrieves info
     *             for all records.
     *  @param[in] Network time out in seconds. If omitted or negative, previous value
     *             will be reused. System default is 10.
     */
    virtual void Populate(const int discnum=-1, const int timeout=-1) {}

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
    static const std::string base_url = "https://api.discogs.com/";

    /** Initialize a new disc and fill it with disc info
         *  from the supplied cuesheet and length. Previously created disc
         *  data are discarded. After disc and its tracks are initialized,
         *  CDDB disc ID is computed. If the computation fails, function
         *  throws an runtime_error.
         *
         *  @param[in] CD-ROM device path
         */
    void InitDisc_(const std::string &dev);

    /** Clear all the disc entries
             */
    void ClearDiscs_();

    std::deque<json_t *root> Releases;
};
