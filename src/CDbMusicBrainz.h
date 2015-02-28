#pragma once

#include <deque>
#include <string>
#include <musicbrainz5/Query.h>

#include "SCueSheet.h"
#include "SDbrBase.h"

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

class CDbMusicBrainz
{
public:
	/** Constructor.
	 *
	 *  @param[in] CDDB server name. If omitted or empty, default server
	 *             "musicbrainz.org" is used.
	 *  @param[in] CDDB server port. If omitted, default value is 80.
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

	/** Returns the CD record ID associated with the specified genre. If no matching
	 *  record is found, it returns -1.
	 *
	 *  @return Matching CD record ID. 
	 */
	virtual int MatchByGenre(const std::string &genre) const;

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
	virtual SDbrBase* Retrieve(const int recnum=0);

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
	 *  @param[in] CD-ROM device path
	 */
	void InitDisc_(const std::string &dev);

	/** Clear all the disc entries
	 */
	void ClearDiscs_();

	std::string discid;
	MusicBrainz5::CQuery MB5;
	std::deque<MusicBrainz5::CRelease> Releases;
};

