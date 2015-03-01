#include "CDbCDDB.h"

#include <stdexcept>
#include <stdlib.h> // for http_proxy environmental variable access

#include <sstream>
#include <iostream>
#include <iomanip>
using std::cout;
using std::endl;

#define ENV_HTTP_PROXY "http_proxy"
#define HTTP_PREFIX "http://"
#define HTTP_PREFIX_LEN 7

using std::ostringstream;
using std::deque;
using std::string;
using std::runtime_error;
using std::to_string;


int CDbCDDB::num_instances = 0;

/** Initialize a new disc and fill it with disc info
 *  from the supplied cuesheet and length. Previously created disc
 *  data are discarded. After disc and its tracks are initialized, 
 *  CDDB disc ID is computed. If the computation fails, function 
 *  throws an runtime_error.
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
CDbCDDB::CDbCDDB(const std::string &servername, const int serverport,
		  const std::string &protocol,  const std::string &email,
		  const std::string &cachemode, const std::string &cachedir,
		  const std::string &cname,const std::string &cversion)
{
	// Create a new connection structure
	conn = cddb_new();

	/* If the pointer is NULL then an error occured (out of memory). */
	if (!conn) throw(runtime_error("Unable to create CDDB connection structure."));

	// set server name
	if (!servername.empty()) cddb_set_server_name(conn, servername.c_str());

	// set server port
	if (serverport>0) cddb_set_server_port(conn,serverport);

	// set connection protocol
	SetProtocol(protocol);

	// set email
	if (!email.empty()) cddb_set_email_address(conn, email.c_str());

	// set Local cache Mode
	SetCacheSettings(cachemode,cachedir);
	
	// set client name
	if (!cname.empty())
	{
		if (cversion.empty()) cddb_set_client(conn, cname.c_str(), "UNKNOWN");
		else cddb_set_client(conn, cname.c_str(), cversion.c_str());
	}

	// increment instance counter
	num_instances++;
}

CDbCDDB::~CDbCDDB()
{
	// Clear disc info collection
	ClearDiscs_();
	
	// destroy the connection
	cddb_destroy(conn);

	// decrement instance counter & if no other instances exist
	// also destroy any global resources reserved by the library.
	if (--num_instances) libcddb_shutdown();
}

/** Set a server connection protocol.
 *
 *  @param[in] Connection protocol ("cddbp","http", or "proxy"). If
 *             "proxy" is specified, "http_proxy" system environmental
 *             variable will be used. If the variable is not set, the
 *             constructor throws an error.
 */
void CDbCDDB::SetProtocol(const std::string &protocol)
{
	if (protocol.empty() || protocol.compare("cddbp") == 0)
	{
		/* Enable the CDDBP protocol, i.e. disable the usage of HTTP.
		This is the default so actually this function call is not
		needed. */
		cddb_http_disable(conn);
	}
	else if (protocol.compare("http") == 0)
	{
		/* Enable the HTTP protocol.  We will also set the server port
		to 80, i.e. the default HTTP port. */
		cddb_http_enable(conn);
		cddb_set_server_port(conn, 80);
	}
	else if (protocol.compare("proxy") == 0)
	{
		/* Enable the HTTP protocol through a proxy server.  Enabling
		the proxy will automatically enable HTTP.  We will also set
		the server port to 80, i.e. the default HTTP port.  */
		cddb_http_proxy_enable(conn);
		cddb_set_server_port(conn, 80);
		
		/* We will retrieve the proxy settings from the environment
		variable 'http_proxy'.  If these do not exist, an error
		will be signaled. */
		string aux(getenv(ENV_HTTP_PROXY));
		if (aux.empty()) throw(runtime_error("Environment variable 'http_proxy' not set"));

		// Breaking down http_proxy variable: http://username:password@host:port

		// Check the prefix
		if (aux.compare(0, HTTP_PREFIX_LEN, HTTP_PREFIX) != 0)
			throw(runtime_error("Environment variable 'http_proxy' is invalid."));

		// store the host address without the prefix
		string host(aux, HTTP_PREFIX_LEN);
		
		/* Check if a proxy username:password pair is provided */
		size_t i = host.find('@');
		if (i!=string::npos)	// @ symbol found (look for username and possibly password)
		{
			string username(host,0,i-1);	// username (may contain password)
			host.erase(0,i); // erase up to @

			i = username.find(':');
			if (i!=string::npos)	// password found
			{
				string password(username,i+1,string::npos);
				username.erase(i,string::npos);
				
				// set both proxy username & password
				cddb_set_http_proxy_credentials(conn, username.c_str(), password.c_str());
			}
			else
			{
				// no password, just set proxy username
				cddb_set_http_proxy_username(conn, username.c_str());
			}
		}
		
		/* Check if a proxy port is specified. */
		i = host.find(':');
		if (i!=string::npos)
		{
			// separate host & port strings
			string port(host,i+1,string::npos);
			host.erase(i,string::npos);

			// set port (convert it to int first)
			// if conversion fails, throws an error
			cddb_set_http_proxy_server_port(conn, std::stol(port));
		}

		// Finally set the proxy server name 
		cddb_set_http_proxy_server_name(conn, host.c_str());
	}
	else
	{
		throw(runtime_error("Invalid CDDB server protocol."));
	}
}

/** Set Local cache settings
 *
 *  @param[in] Local cache mode: "on", "off", or "only". If empty, left unchanged.
 *  @param[in] Local cache directory. If empty, left unchanged.
 */
void CDbCDDB::SetCacheSettings(const std::string &cachemode, const std::string &cachedir)
{
	if (!cachemode.empty())
	{
		if (cachemode.compare("on")==0) cddb_cache_enable(conn);
		else if (cachemode.compare("off")==0) cddb_cache_disable(conn);
		else if (cachemode.compare("only")==0) cddb_cache_only(conn);
		else throw(runtime_error("Invalid cache mode."));
	}

	// set cache directory
	if (!cachedir.empty()) cddb_cache_set_dir(conn, cachedir.c_str());
}

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
int CDbCDDB::Query(const std::string &dev, const SCueSheet &cuesheet, const size_t len,
						const bool autofill, const int timeout)
{
	// must build disc based on cuesheet (throws error if fails to compute discid)
	InitDisc_(cuesheet, len);

	// Set timeout if specified
	if (timeout>0) cddb_set_timeout(conn,timeout);

	// Run the query
	int matches = cddb_query(conn, discs[0]);

	// If errored out, throw the error
	if (matches<0) throw(runtime_error(cddb_error_str(cddb_errno(conn))));

	// store all the matches 
	for (int i=1;i<matches;i++)
	{
		// create a new disc object
		cddb_disc_t *disc = cddb_disc_clone(discs[0]);

		// populate the disc, throw error if it failed
		if (!cddb_query_next(conn, disc))
			throw(runtime_error(cddb_error_str(cddb_errno(conn))));

		// store it in the deque
		discs.push_back(disc);
	}
	
	// Populate the records if requested
	if (autofill) Populate(-1,timeout);
	
	// return the number of matches
	return matches;
}

/** Return the CDDB discid string
 *
 *  @return CDDB discid (8 hexdigits) if Query() has been completed successfully. 
 *          Otherwise "00000000".
 */
std::string CDbCDDB::GetDiscId() const
{
	if (discs.empty()) return string("00000000");

	ostringstream os;
	os << std::setfill ('0') << std::setw(8) << std::hex << cddb_disc_get_discid(discs[0]);
	
	return os.str();
}

/** Returns the number of matches (records) returned from the last Query() call.
 *
 *  @return    Number of matches
 */
int CDbCDDB::NumberOfMatches() const
{
	return discs.size();
};

/** Get album title
*
*  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
*             is returned.
*  @return    Title string (empty if title not available)
*/
virtual std::string CDbCDDB::AlbumTitle(const int recnum=0) const
{
    // set disc
    if (recnum<0 || recnum>=(int)discs.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return cddb_disc_get_title(discs[recnum]);
}

/** Get album artist
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Artist string (empty if artist not available)
 */
virtual std::string CDbCDDB::AlbumArtist(const int recnum=0) const
{
    // set disc
    if (recnum<0 || recnum>=(int)discs.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return cddb_disc_get_artist(discs[recnum]);	// performer (80-char long max)
}

/** Get genre
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Genre string (empty if genre not available)
 */
virtual std::string CDbCDDB::Genre(const int recnum=0) const
{
    // set disc
    if (recnum<0 || recnum>=(int)discs.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return cddb_disc_get_genre(discs[recnum]);
}

/** Returns the CD record ID associated with the specified genre. If no matching
	 *  record is found, it returns -1.
	 *
	 *  @return Matching CD record ID. -1 if no match found.
	 */
int CDbCDDB::MatchByGenre(const std::string &genre) const
{
	deque<cddb_disc_t*>::const_iterator it;
	for (it=discs.begin(); it!=discs.end(); it++)
	{
		// compare to both textual Genre and CDDB genre category
		if (genre.compare(cddb_disc_get_genre(*it))==0 ||
			 genre.compare(cddb_disc_get_category_str(*it))==0)
		{
			return it-discs.begin();
		}
	}		

	return -1;
}

/** Look up full disc information from CDDB server. It supports single record or
 *  multiple records if multiple entries were found by Query(). If the computation
 *  fails, function throws an runtime_error.
 *
 *  @param[in] Disc record ID (0-based index to discs). If negative, retrieves info
 *             for all records.
 *  @param[in] Network time out in seconds. If omitted or negative, previous value
 *             will be reused. System default is 10.
 */
void CDbCDDB::Populate(const int recnum, const int timeout)
{
	// Set timeout if specified
	if (timeout>0) cddb_set_timeout(conn,timeout);

	// set disc
	if (recnum<0) // all discs
	{
		deque<cddb_disc_t*>::iterator it;
		for (it=discs.begin(); it!=discs.end(); it++)
		{
			if (cddb_read(conn,*it)!=1)
				throw(runtime_error(cddb_error_str(cddb_errno(conn))));
		}		
	}
	else if (recnum<(int)discs.size())	// single disc
	{
		if (cddb_read(conn,discs[recnum])!=1)
			throw(runtime_error(cddb_error_str(cddb_errno(conn))));
	}
	else
	{
		throw(runtime_error("Invalid CD record ID."));
	}
}


/** Retrieve the disc info from specified database record
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    SDbrBase Pointer to newly created database record object. Caller is
 *             responsible for deleting the object.
 */
SDbrBase* CDbCDDB::Retrieve(const int recnum)
{
	const char *str;	// temp

	// set disc
	if (recnum<0 || recnum>=(int)discs.size()) // all discs
		throw(runtime_error("Invalid CD record ID."));

	// grab the disc info
	cddb_disc_t *disc = discs[recnum];

	// instantiate new DBR object
	SDbrCDDB * rec = new SDbrCDDB;

	// populate the disc info (use REM for non-essential data)
	rec->Performer = cddb_disc_get_artist(disc);	// performer (80-char long max)
	rec->Title = cddb_disc_get_title(disc);
	
	rec->Rems.emplace_back("SRCDB FreeDB");	// source database
	rec->Rems.emplace_back("DISCID ");	// comments on the disc 
	rec->Rems[1].append(GetDiscId()); 

	rec->Rems.emplace_back("LENGTH ");
	rec->Rems[2].append(to_string(cddb_disc_get_length(disc)));
	
	rec->Rems.emplace_back("GENRE ");
	str = cddb_disc_get_genre(disc);
	if (str) rec->Rems[3].append(cddb_disc_get_genre(disc));
	else rec->Rems[3].append(cddb_disc_get_category_str(disc));

	rec->Rems.emplace_back("DATE ");
	rec->Rems[4].append(to_string(cddb_disc_get_year(disc)));

	str = cddb_disc_get_ext_data(disc);
	if (str) rec->Rems.emplace_back(str);

	// initialize tracks
	int num_tracks = cddb_disc_get_track_count(disc);
	rec->AddTracks(num_tracks); // adds Tracks 1 to num_tracks
	
	// for each track
	cddb_track_t * track = cddb_disc_get_track_first (disc);
	for (int i = 1; i <= num_tracks ; i++)
	{
		if (track==NULL)
			throw(runtime_error(cddb_error_str(cddb_errno(conn))));
	
		// get the track object
		SCueTrack &rectrack = rec->Tracks[i-1];

		// add Index 1 with the start time
		rectrack.AddIndex(1,cddb_track_get_frame_offset(track));
		
		rectrack.Title = cddb_track_get_title(track);
		rectrack.Performer = cddb_track_get_artist(track);

		str = cddb_track_get_ext_data(track);
		if (str) rectrack.Rems.emplace_back(str);

		if (i!= num_tracks) track = cddb_disc_get_track_next (disc);
	}
	
	return rec;
}

/** Clear all the disc entries
 */
void CDbCDDB::ClearDiscs_()
{
	// destroy the discs
	deque<cddb_disc_t*>::iterator it;
	for (it=discs.begin();it!=discs.end();it++) cddb_disc_destroy(*it);
	
	// clear the collection
	discs.clear();	 
}

/** Initialize a new disc and fill it with disc info
 *  from the supplied cuesheet and length. Previously created disc
 *  data are discarded. After disc and its tracks are initialized, 
 *  CDDB disc ID is computed. If the computation fails, function 
 *  throws an runtime_error.
 *
 *  @param[in] Cuesheet with its basic data populated
 *  @param[in] Length of the CD in sectors
 */
void CDbCDDB::InitDisc_(const SCueSheet &cuesheet, const size_t len)
{
	// Clear the discs
	ClearDiscs_();

	// Create a disc
	cddb_disc_t* disc = cddb_disc_new();
	discs.push_front(disc);	// store the new disc
	
	// Set the disc length in seconds
	cddb_disc_set_length(disc, len/FRAMES_PER_SECOND);

	// Create its tracks
	deque<SCueTrack>::const_iterator it;
	for (it=cuesheet.Tracks.begin(); it!=cuesheet.Tracks.end(); it++)
	{
		// create a new track
		cddb_track_t *track = cddb_track_new();
		
		// add it to the disc
		cddb_disc_add_track(disc,track);
		
		// set its offset in seconds
		cddb_track_set_frame_offset(track, (*it).StartTime());
	}

	// Populate the discid
	if (!cddb_disc_calc_discid(disc)) 
		throw(runtime_error("Failed to compute CDDB disc ID."));

}

void CDbCDDB::Print(const int recnum) const
{
	if (discs.empty()) cout << "No match found" << endl;

	if (recnum<0) // all discs
	{
		cout << "Found " << discs.size() << " matches:" << endl;
		deque<cddb_disc_t*>::const_iterator it;
		for (it=discs.begin(); it!=discs.end(); it++)
		{
			cddb_disc_t* disc = *it;

			// Retrieve and print the category and disc ID.
			cout << endl << "  category: "<< cddb_disc_get_category(disc) << " ("
				  << GetDiscId() << ") " 
				  << cddb_disc_get_category_str(disc) << endl;

			// Retrieve and print the disc title and artist name.
			cout << "  '" << cddb_disc_get_title(disc) << "' by " << cddb_disc_get_artist(disc) << endl;
		}
	}
	else if (recnum<(int)discs.size())	// single disc
	{
		cddb_disc_print(discs[recnum]);
	}
	else
	{
		throw(runtime_error("Invalid CD record ID."));
	}
}

