#pragma once

#include <deque>
#include <string>
#include <musicbrainz5/Query.h>
#include <musicbrainz5/ArtistCredit.h>
#include <coverart/CoverArt.h>

#include "IDatabase.h"
#include "ICoverArt.h"
#include "SDbrBase.h"

struct SCueSheet;
class CDbDiscogs;

/** MusicBrainz CD Database Record structure - SCueSheet with DbType()
 */
struct SDbrMusicBrainz : SDbrBase
{
    /** Name of the database the record was retrieved from
   *
   *  @return Name of the database
   */
    virtual std::string SourceDatabase() const { return "MusicBrainz"; }

    friend std::ostream& operator<<(std::ostream& stdout, const SDbrMusicBrainz& obj);
};

/** Overloaded stream insertion operator to output the content of SDbrMusicBrainz
 *  object. The output is in accordance with the CDRWIN's Cue-Sheet syntax
 *
 *  @param[in]  Reference to an std::ostream object
 *  @return     Copy of the stream object
 */
inline std::ostream& operator<<(std::ostream& os, const SDbrMusicBrainz& o)
{
    os << dynamic_cast<const SCueSheet&>(o);
    return os;
}

/** Class to access MusicBrainz online CD and coverart databases service.
 */
class CDbMusicBrainz : public CDbBase, public IDatabase, public ICoverArt
{
public:
    /** Constructor.
     *
     *  @param[in] Server name. If omitted or empty, default server
     *             "musicbrainz.org" is used.
     *  @param[in] Server port. If omitted, default value is 80.
     *  @param[in] MusicBrainz account name for tagging. If omitted or empty, no
     *             action is taken.
     *  @param[in] MusicBrainz account password for tagging. If omitted or empty
     *             no action is taken.
     *  @param[in] Client program name. If omitted or empty, uses "autorip"
     *  @param[in] Client program version. If omitted or empty, uses "alpha"
     */
    CDbMusicBrainz(const std::string &servername="musicbrainz.org", const int serverport=80,
                   const std::string &username="", const std::string &password="",
                   const std::string &cname="autorip",const std::string &cversion="alpha");

    /** Destructor
     */
    virtual ~CDbMusicBrainz();

    /**
     * @brief Return true if Database can be searched for a CD info
     * @return true if database can be searched for a CD release
     */
    bool IsReleaseDb() { return true; }

    /**
     * @brief Return true if Database can be searched for a CD cover art
     * @return true if database can be searched for a CD cover art
     */
    bool IsImageDb() { return true; }

    /**
     * @brief Return database type enum
     * @return ReleaseDatabase enumuration value
     */
    virtual ReleaseDatabase GetDatabaseType() const { return ReleaseDatabase::MUSICBRAINZ; }

    /** Always return true as MusicBrainz query is supported.
     *
     *  @return    true if query is supported
     */
    static bool IsQueryable() const { return true; }

    /** Always return false as MusicBrainz's search feature is not implemented.
     *
     *  @return    true if search is supported
     */
    static bool IsSearchable() const { return false; }

    /** Set a server connection protocol.
     *
     *  @param[in] Connection protocol ("cddbp","http", or "proxy"). If omitted
     *             or empty, 'cddbp' is used by the default. If "proxy" is
     *             specified, "http_proxy" system environmental variable will
     *             be used.
     */
    void SetProtocol(const std::string &protocol);

    /** Perform a new MusicBrainz query for the disc info given in the supplied cuesheet
     *  and its length. Previous query outcome discarded. After disc and its tracks are initialized,
     *  CDDB disc ID is computed. If the computation fails, function
     *  throws an runtime_error.
     *
     *  After successful Query() call, NumberOfMatches() and
     *
     *  @param[in] CD-ROM device path
     *  @param[in] Cuesheet with its basic data populated (not used)
     *  @param[in] Length of the CD in seconds (not used)
     *  @param[in] If true, immediately calls Read() to populate disc records.
     *  @param[in] Network time out in seconds. (not used)
     *  @return    Number of matched records
     */
    virtual int Query(const std::string &dev, const SCueSheet &cuesheet, const size_t len, const bool autofill=false, const int timeout=-1);

    /** Searching MusicBrainz database by album title and artist is not implemented. Thus, always returns 0.
     *
     *  @param[in] Album title
     *  @param[in] Album artist
     *  @param[in] If true, immediately calls Read() to populate disc records.
     *  @param[in] Network time out in seconds. If omitted or negative, previous value
     *             will be reused. System default is 10.
     *  @return    Number of matched records
     */
    virtual int Search(const std::string &title, const std::string &artist, const bool autofill=false, const int timeout=-1)
    { return 0; }

    /** Return the MusicBrainz discid string
     *
     *  @return MusicBrainz discid string if Query was successful. Otherwise "00000000".
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
     *  @return    Empty string (MusicBrainz does not support genre)
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

    /** Get album ASIN (Amazon Standard Identification Number)
     *
     *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
     *             is returned.
     *  @return    ASIN string (empty if title not available)
     */
    virtual std::string AlbumASIN(const int recnum=-1) const;

    /** Returns the CD record ID associated with the specified genre. If no matching
     *  record is found, it returns -1.
     *
     *  @return Matching CD record ID.
     */
    virtual int MatchByGenre(const std::string &genre) const { return -1; }

    /** Look up full disc information from MusicBrainz server. It supports single record or
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

    /** Specify the preferred coverart image width
     *
     *  @param[in] Preferred width of the image
     */
    virtual void SetPreferredWidth(const size_t &width);

    /** Specify the preferred coverart image height
     *
     *  @param[in] Preferred height of the image
     */
    virtual void SetPreferredHeight(const size_t &height);

    /** Check if the query returned a front cover
     *
     *  @param[in]  record index (default=0)
     *  @return     true if front cover is found.
     */
    virtual bool Front(const int recnum=0) const;

    /** Check if the query returned a back cover
     *
     *  @param[in]  record index (default=0)
     *  @return     true if back cover is found.
     */
    virtual bool Back(const int recnum=0) const;

    /** Retrieve the front cover data.
     *
     *  @param[out] image data buffer.
     *  @param[in]  record index (default=0)
     */
    virtual std::vector<unsigned char> FrontData(const int recnum=0) const;

    /** Check if the query returned a front cover
     *
     *  @param[out] image data buffer.
     *  @param[in]  record index (default=0)
     */
    virtual std::vector<unsigned char> BackData(const int recnum=0) const;

    /** Get the URL of the front cover image
     *
     *  @param[in]  Record index (default=0)
     *  @return     URL string
     */
    virtual std::string FrontURL(const int recnum=0) const;

    /** Get the URL of the back cover image
     *
     *  @param[in]  Record index (default=0)
     *  @return     URL string
     */
    virtual std::string BackURL(const int recnum=0) const;

    /** Return the MusicBrainz release ID string
     *
     *  @return MusicBrainz release ID string if Query was successful. Otherwise "".
     */
    virtual std::string GetReleaseId(const int recnum=0) const;

    /** Spawn a Discogs Database object based on the current query results. The Discogs
     *  object is created for every MusicBrainz queried release, which contains a link to
     *  http://www.discogs.com/release/*
     *
     *  @return Discogs databased object
     */
    //virtual CDbDiscogs SpawnDiscogs() const;

private:
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

    /** Form an artist credit string
     *
     *  @param[in] ArtistCredit query data
     *  @param[in] true to use SortName (if available) for the first asrtist's name
     *             (default is false)
     *  @return    Artist name in plain string
     */
    std::string GetArtistString_(const MusicBrainz5::CArtistCredit &credit, const bool sortfirst=false) const;

    std::string discid;
    MusicBrainz5::CQuery MB5;
    CoverArtArchive::CCoverArt CAA;
    std::deque<MusicBrainz5::CRelease> Releases;
    std::deque<CoverArtArchive::CReleaseInfo> CoverArts;

    int CoverArtSize; // 0-full, 1-large thumbnail (500px), 2-small thumbnail (250px)
};
