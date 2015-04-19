#include "CDbFreeDb.h"

#include <stdexcept>
#include <stdlib.h> // for http_proxy environmental variable access

#include "SCueSheet.h"

#include <sstream>
#include <iostream>
#include <iomanip>
using std::cout;
using std::endl;

#define ENV_HTTP_PROXY "http_proxy"
#define HTTP_PREFIX "http://"
#define HTTP_PREFIX_LEN 7
#define PREGAP_OFFSET 2*75 // in sectors

using std::ostringstream;
using std::vector;
using std::string;
using std::runtime_error;
using std::to_string;

int CDbFreeDb::num_instances = 0;

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
CDbFreeDb::CDbFreeDb(const std::string &servername, const int serverport,
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

    // Set timeout if specified
    //if (timeout>0) cddb_set_timeout(conn,timeout);

    // increment instance counter
	num_instances++;
}

CDbFreeDb::~CDbFreeDb()
{
	// Clear disc info collection
    Clear();
	
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
void CDbFreeDb::SetProtocol(const std::string &protocol)
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
void CDbFreeDb::SetCacheSettings(const std::string &cachemode, const std::string &cachedir)
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


/** If AllowQueryCD() returns true, Query() performs a new query for the CD info
 *  in the specified drive with its *  tracks specified in the supplied cuesheet
 *  and its length. Previous query outcome discarded. After disc and its tracks
 *  are initialized, CDDB disc ID is computed. If the computation fails, function
 *  throws an runtime_error.
 *
 *  @param[in] CD-ROM device path
 *  @param[in] Cuesheet with its basic data populated
 *  @param[in] Length of the CD in sectors
 *  @param[in] (Optional) UPC barcode
 *  @return    Number of matched records
 */
int CDbFreeDb::Query(const SCueSheet &cuesheet, const std::string upc)
{
    cddb_disc_t *disc;

	// must build disc based on cuesheet (throws error if fails to compute discid)
    InitDisc_(cuesheet);

	// Run the query
    int matches = cddb_query(conn, disc);

	// If errored out, throw the error
	if (matches<0) throw(runtime_error(cddb_error_str(cddb_errno(conn))));

    // Obtain the full record of the first disc
    if (cddb_read(conn,disc)!=1)
        throw(runtime_error(cddb_error_str(cddb_errno(conn))));

    // reserve space for the matches
    discs.reserve(matches);
    discs.push_back(disc);

	for (int i=1;i<matches;i++)
	{
		// create a new disc object
        disc = cddb_disc_clone(discs[0]);

		// populate the disc, throw error if it failed
        if (!(cddb_query_next(conn, disc) && cddb_read(conn,disc)==1))
			throw(runtime_error(cddb_error_str(cddb_errno(conn))));

        // store it in the vector
		discs.push_back(disc);
	}
	
    // return the number of matches
	return matches;
}

/** Return the CDDB discid string
 *
 *  @return CDDB discid (8 hexdigits) if Query() has been completed successfully. 
 *          Otherwise "00000000".
 */
std::string CDbFreeDb::GetDiscId() const
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
int CDbFreeDb::NumberOfMatches() const
{
	return discs.size();
};

/** Return a unique release ID string
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return id string if Query was successful.
 */
std::string CDbFreeDb::ReleaseId(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)discs.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return GetDiscId();
}

/** Get album title
*
*  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
*             is returned.
*  @return    Title string (empty if title not available)
*/
std::string CDbFreeDb::AlbumTitle(const int recnum) const
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
SCueArtists CDbFreeDb::AlbumArtist(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)discs.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    SCueArtists rval;

    std::string name = cddb_disc_get_artist(discs[recnum]);
    if (name.size())
    {
        rval.emplace_back();
        rval.back().name = name;
    }

    return rval;
}

/** Get album composer
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Composer/songwriter string (empty if artist not available)
 */
SCueArtists CDbFreeDb::AlbumComposer(const int recnum) const
{
    SCueArtists rval;
    return rval;
}

/** Get genre
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Genre string (empty if genre not available)
 */
std::string CDbFreeDb::Genre(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)discs.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return cddb_disc_get_genre(discs[recnum]);
}

/** Get release date
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Date string (empty if genre not available)
 */
std::string CDbFreeDb::Date(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)discs.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return to_string(cddb_disc_get_year(discs[recnum]));
}

/** Get number of tracks
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    number of tracks
 *  @throw     runtime_error if CD record id is invalid
 */
int CDbFreeDb::NumberOfTracks(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)discs.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    // grab the disc info
    return cddb_disc_get_track_count(discs[recnum]);
}

/** Get track title
 *
 *  @param[in] Track number (1-99)
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Title string (empty if title not available)
 *  @throw     runtime_error if track number is invalid
 */
std::string CDbFreeDb::TrackTitle(int tracknum, const int recnum) const
{
    // check disc
    if (recnum<0 || recnum>=(int)discs.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    // check track
    if (tracknum<=0 || tracknum>NumberOfTracks()) // all discs
        throw(runtime_error("Invalid CD Track Number."));

    // grab the first track
    cddb_track_t * track = cddb_disc_get_track_first (discs[recnum]);

    // go to the track
    for (int i = 1; i != tracknum ; i++)
        track = cddb_disc_get_track_next (discs[recnum]);

    return cddb_track_get_title(track);
}

/** Get track artist
 *
 *  @param[in] Track number (1-99)
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Artist string (empty if artist not available)
 *  @throw     runtime_error if track number is invalid
 */
SCueArtists CDbFreeDb::TrackArtist(int tracknum, const int recnum) const
{
    // check disc
    if (recnum<0 || recnum>=(int)discs.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    // check track
    if (tracknum<=0 || tracknum>NumberOfTracks()) // all discs
        throw(runtime_error("Invalid CD Track Number."));

    // grab the first track
    cddb_track_t * track = cddb_disc_get_track_first (discs[recnum]);

    // go to the track
    for (int i = 1; i != tracknum ; i++)
        track = cddb_disc_get_track_next (discs[recnum]);

    SCueArtists rval;
    std::string name = cddb_track_get_artist(track);
    if (name.size())
    {
        rval.emplace_back();
        rval.back().name = name;
    }

    return rval;
}

/** Get track composer
 *
 *  @param[in] Track number (1-99)
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Composer string (empty if artist not available)
 *  @throw     runtime_error if track number is invalid
 */
SCueArtists CDbFreeDb::TrackComposer(int tracknum, const int recnum) const
{
    SCueArtists rval;
    return rval;
}

/** Clear all the disc entries
 */
void CDbFreeDb::Clear()
{
	// destroy the discs
    vector<cddb_disc_t*>::iterator it;
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
void CDbFreeDb::InitDisc_(const SCueSheet &cuesheet)
{
	// Clear the discs
    Clear();

	// Create a disc
	cddb_disc_t* disc = cddb_disc_new();
    discs.push_back(disc);	// store the new disc
	
	// Set the disc length in seconds
    cddb_disc_set_length(disc, (cuesheet.TotalTime+150)/FRAMES_PER_SECOND);

	// Create its tracks
    SCueTrackDeque::const_iterator it;
	for (it=cuesheet.Tracks.begin(); it!=cuesheet.Tracks.end(); it++)
	{
		// create a new track
		cddb_track_t *track = cddb_track_new();
		
		// add it to the disc
		cddb_disc_add_track(disc,track);
		
		// set its offset in seconds
        cddb_track_set_frame_offset(track, (*it).StartTime()+PREGAP_OFFSET);
	}

	// Populate the discid
	if (!cddb_disc_calc_discid(disc)) 
		throw(runtime_error("Failed to compute CDDB disc ID."));

}
