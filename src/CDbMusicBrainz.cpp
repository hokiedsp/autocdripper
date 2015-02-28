/**
 * Libraries: libmusicbrainz5, libdiscid
 */

#include "CDbMusicBrainz.h"

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <iomanip>

#include <musicbrainz5/Release.h>
#include <musicbrainz5/Medium.h>
#include <musicbrainz5/MediumList.h>
#include <musicbrainz5/ReleaseGroup.h>
#include <musicbrainz5/Track.h>
#include <musicbrainz5/Recording.h>
#include <musicbrainz5/NameCredit.h>
#include <musicbrainz5/Artist.h>
#include <musicbrainz5/Label.h>
#include <musicbrainz5/LabelInfo.h>
#include <musicbrainz5/ISRC.h>

#include <musicbrainz5/TextRepresentation.h>
#include <musicbrainz5/ArtistCredit.h>
#include <musicbrainz5/RelationListList.h>
#include <musicbrainz5/Relation.h>

#include <discid/discid.h>


using std::cout;
using std::endl;

using std::ostringstream;
using std::deque;
using std::string;
using std::runtime_error;
using std::to_string;

using namespace MusicBrainz5;

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
CDbMusicBrainz::CDbMusicBrainz(const std::string &servername, const int serverport,
										 const std::string &username, const std::string &password,
										 const std::string &cname,const std::string &cversion):
	MB5(cname+"-"+cversion,servername,serverport)
{
	if (!username.empty()) MB5.SetUserName(username);
	if (!password.empty()) MB5.SetPassword(password);
}

CDbMusicBrainz::~CDbMusicBrainz()
{
}

/** Perform a new CDDB query for the disc info given in the supplied cuesheet 
 *  and its length. Previous query outcome discarded. After disc and its tracks are initialized, 
 *  CDDB disc ID is computed. If the computation fails, function 
 *  throws an runtime_error.
 *
 *  After successful query, disc's genre, artist, and title should be filled.
 *
 *  @param[in] Cuesheet with its basic data populated
 *  @param[in] Length of the CD in seconds
 *  @param[in] Network time out in seconds. CDbMusicBrainz does not use this parameter
 *  @return    Number of matches
 */
int CDbMusicBrainz::Query(const std::string &dev, const SCueSheet &cuesheet, const size_t len, const bool autofill, const int timeout)
{
	// redirect cerr stream during this function call
	cerr_redirect quiet_cerr;

	// must build disc based on cuesheet (throws error if fails to compute discid)
	InitDisc_(dev);

	// Run the query
	CReleaseList list = MB5.LookupDiscID(discid);
	
	// Add all the matched releases to Releases deque
	for (int i=0;i<list.NumItems();i++)
	{
		CRelease *r = list.Item(i);
		if (r->MediaMatchingDiscID (discid).NumItems()) Releases.emplace_back(*r);
	}
	
	// Populate the records if requested
	if (autofill) Populate(-1);
	
	// return the number of matches
	return Releases.size();
}

/** Return the CDDB discid
 *
 *  @return CDDB discid if Query() has been completed successfully. Otherwise
 *          zero.
 */
std::string CDbMusicBrainz::GetDiscId() const
{
	return discid;
}


/** Returns the number of matches (records) returned from the last Query() call.
 *
 *  @return    Number of matches
 */
int CDbMusicBrainz::NumberOfMatches() const
{
	return Releases.size();
};

/** Returns the CD record ID associated  with the specified genre. If no matching
 *  record is found, it returns -1.
 *
 *  @return Matching CD record ID. -1 if no match found.
 */
int CDbMusicBrainz::MatchByGenre(const std::string &genre) const
{
	return -1;
}

/** Look up full disc information from MusicBrainz server. It supports single record or
 *  multiple records if multiple entries were found by Query(). If the computation
 *  fails, function throws an runtime_error.
 *
 *  @param[in] Disc record ID (0-based index to discs). If negative, retrieves info
 *             for all records.
 *  @param[in] Network time out in seconds. CDbMusicBrainz does not use this parameter
 */
void CDbMusicBrainz::Populate(const int recnum, const int timeout)
{
	// redirect cerr stream during this function call
	cerr_redirect quiet_cerr;

	// set disc
	if (recnum<0) // all discs
	{
		deque<CRelease>::iterator it;
		for (it=Releases.begin(); it!=Releases.end(); it++)
			(*it) = MB5.LookupRelease((*it).ID()); 	// replace the initial output with the full output
	}
	else if (recnum<(int)Releases.size())	// single disc
	{
		CRelease &r = Releases[recnum];
		r = MB5.LookupRelease(r.ID()); 	// replace the initial output with the full output
	}
	else
	{
		throw(runtime_error("Invalid Release ID."));
	}
}

/** Retrieve the disc info from specified database record
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    SDbrBase Pointer to newly created database record object. Caller is
 *             responsible for deleting the object.
 */
SDbrBase* CDbMusicBrainz::Retrieve(const int recnum)
{
	//const char *str;	// temp
	//ostringstream stream; // for formatting

	// instantiate new DBR object
	SDbrMusicBrainz * rec = new SDbrMusicBrainz;

/*
	// set disc
	if (recnum<0 || recnum>=(int)discs.size()) // all discs
		throw(runtime_error("Invalid CD record ID."));

	// populate the disc info (use REM for non-essential data)
	// grab the disc info
	cddb_disc_t *disc = discs[recnum];

	rec->Performer = cddb_disc_get_artist(disc);	// performer (80-char long max)
	rec->Title = cddb_disc_get_title(disc);
	
	rec->Rems.emplace_back("DISCID ");	// comments on the disc 
	stream << std::setfill ('0') << std::setw(8) << std::hex << cddb_disc_get_discid(disc);
	rec->Rems[0].append(stream.str()); 

	rec->Rems.emplace_back("LENGTH ");
	rec->Rems[1].append(to_string(cddb_disc_get_length(disc)));
	
	rec->Rems.emplace_back("GENRE ");
	str = cddb_disc_get_genre(disc);
	if (str) rec->Rems[2].append(cddb_disc_get_genre(disc));
	else rec->Rems[2].append(cddb_disc_get_category_str(disc));

	rec->Rems.emplace_back("DATE ");
	rec->Rems[3].append(to_string(cddb_disc_get_year(disc)));

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
	*/
	return rec;
}

/** Clear all the disc entries
 */
void CDbMusicBrainz::ClearDiscs_()
{
	// clear the list of matched releases
	Releases.clear();
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
void CDbMusicBrainz::InitDisc_(const std::string &path)
{

	// Clear the discs
	ClearDiscs_();

	// Get discid string
	DiscId *did = discid_new();

	if (discid_read_sparse(did,path.c_str(),0)== 0)
	{
		discid_free(did); 
		throw(runtime_error(discid_get_error_msg(did)));
	}

	discid = string(discid_get_id(did));
	discid_free(did); 

}

void CDbMusicBrainz::Print(const int recnum) const
{
	if (Releases.empty()) cout << "No match found" << endl;

	if (recnum<0) // all discs
	{
		cout << "Found " << Releases.size() << " matches:" << endl;
		deque<CRelease>::const_iterator it;
		for (it=Releases.begin(); it!=Releases.end(); it++)
		{
			const CRelease &r = *it;

			// Retrieve and print the category and disc ID.
			cout << r.Title() << " (";
			
			// if multi-disc release, insert the disc position
			if (r.MediumList ()->NumItems()>1)
				cout << r.MediaMatchingDiscID(discid).Item(0)->Position() 
				     << " out of " << r.MediumList ()->NumItems() << ") (";
			
			if (r.Date().empty()) cout << "Unknown";
			else cout << r.Date ();
			cout << ":";
			if (!r.TextRepresentation() || r.TextRepresentation()->Language().empty()) cout << "UNKNOWN";
			else cout << r.TextRepresentation ()->Language();
			cout << ")" << endl;
		}
	}
	else if (recnum<(int)Releases.size())	// single disc
	{
		const CRelease &r = Releases[recnum];
	
		cout << " ID: " << r.ID() << endl;
		cout << " Title: " << r.Title() << endl;
		cout << " Status: " << r.Status () << endl;
		cout << " Quality: " << r.Quality () << endl;
		cout << " Disambiguation: " << r.Disambiguation () << endl;
		cout << " Packaging: " << r.Packaging () << endl;
		cout << " TextRepresentation/Language: " << r.TextRepresentation ()->Language() << endl;	
		cout << " TextRepresentation/Script: " << r.TextRepresentation ()->Script() << endl;	
		if (r.ArtistCredit ())
		{
			CNameCreditList * list = r.ArtistCredit()->NameCreditList ();
			cout << " ArtistCredit: There are " << list->NumItems() << " credits." << endl;
			for (int i=0;i<list->NumItems();i++)
			{
				CNameCredit *credit = list->Item(i);
				cout << "  JoinPhrase: " << credit->JoinPhrase() << endl;
				cout << "  Name: " << credit->Name() << endl;
				if (credit->Artist())
					cout << "  Artist Name: " << credit->Artist()->Name() << endl;
			}
		}
		if (r.ReleaseGroup ())
		{
			cout << " Release Group ID: " << r.ReleaseGroup ()->ID() << endl;
			cout << "  Primary Type: " << r.ReleaseGroup()->PrimaryType () << endl;
			cout << "  Title: " << r.ReleaseGroup()->Title () << endl;
			cout << "  Disambiguation: " << r.ReleaseGroup()->Disambiguation () << endl;
			cout << "  First Release Date: " << r.ReleaseGroup()->FirstReleaseDate () << endl;
		}
		cout << " Date: " << r.Date () << endl;
		cout << " Country: " << r.Country () << endl;
		cout << " Barcode: " << r.Barcode () << endl;
		cout << " ASIN: " << r.ASIN () << endl;
		
		if (r.LabelInfoList ())
		{
			for (int i=0;i<r.LabelInfoList ()->NumItems();i++)
			{
				CLabel *info = r.LabelInfoList ()->Item(i)->Label();
				cout << " Label " << i << ": " << info->ID () << endl;
				cout << "  Type " << info->Type () << endl;
				cout << "  Name " << info->Name () << endl;
				cout << "  Sort Name " << info->SortName () << endl;
				cout << "  Label Code " << info->LabelCode () << endl;
				cout << "  Disambiguation " << info->Disambiguation () << endl;
				cout << "  Country " << info->Country () << endl;
			}
		}
			
		if (r.MediumList ())
			cout << " MediumList Items: " << r.MediumList ()->NumItems() << endl; // total discs
			
		if (r.RelationListList()) // link to amazon or discog
		{
			for (int i = 0;i<r.RelationListList ()->NumItems(); i++)
			{
				CRelationList *list = r.RelationListList ()->Item(i);
				cout << " Relation List" << i << ": " << list->TargetType()  << endl;
				for (int j = 0;j<list->NumItems();j++)
				{
					CRelation *rel = list->Item(j);
					cout << "  Relation " << j << ": " << rel->Type() << endl;
					cout << "   Target: " << rel->Target() << endl;
				}
			}
		}
			
		if (r.CollectionList())
			cout << " CollectionList Items: " << r.CollectionList ()->NumItems() << endl;
		cout << endl;

		CMediumList media = r.MediaMatchingDiscID(discid);
		if (media.NumItems()>1) cout << "***** MULTIPLE MEDIA FOUND *****" << endl;
		
		CMedium *medium = media.Item(0);
		
		cout << " Medium Info" << endl;
		cout << "  Title: " << medium->Title() << endl;
		cout << "  Position: " << medium->Position() << endl;
		cout << "  Format: " << medium->Format() << endl;
		if (medium->DiscList())
			cout << "  DiscList NumItems: " << medium->DiscList()->NumItems() << endl;
		if (medium->TrackList())
		{
			for (int i=0;i<medium->TrackList()->NumItems();i++)
			{
				CTrack *track = medium->TrackList()->Item(i);
				cout << "  Track " << track->Number() << endl;
				cout << "   Position: " << track->Position() << endl;
				cout << "   Title: " << track->Title() << endl;
				cout << "   Length: " << track->Length () << endl;
				
				if (track->Recording())
				{
					cout << "   Recording Title: " << track->Recording()->Title() << endl;
					cout << "   Recording Length: " << track->Recording()->Length() << endl;
					cout << "   Recording Artists " << endl;

					CNameCreditList * list = track->Recording()->ArtistCredit()->NameCreditList ();
					for (int j=0;j<list->NumItems();j++)
					{
						CNameCredit *credit = list->Item(j);
						if (credit->Artist())
						{
							cout << "    Name: " << credit->Artist()->Name() << endl;
							cout << "    SortName: " << credit->Artist()->SortName() << endl;
						}
						if (!credit->JoinPhrase().empty())
							cout << "   JoinPhrase: " << credit->JoinPhrase() << endl;
					}

					CISRCList *isrcs = track->Recording()->ISRCList ();
					if (isrcs)
						for (int j=0;j<isrcs->NumItems();j++)
							cout << "   Recording ISRC: " << isrcs->Item(j)->ID() << endl;
				}
				
				if (track->ArtistCredit())
				{
					CNameCreditList * list = track->ArtistCredit()->NameCreditList ();
					for (int j=0;j<list->NumItems();j++)
					{
						cout << "   Artist " << endl;
						CNameCredit *credit = list->Item(j);
						if (credit->Artist())
						{
							cout << "   Artist Name: " << credit->Artist()->Name() << endl;
							cout << "   Artist SortName: " << credit->Artist()->SortName() << endl;
						}
						if (!credit->JoinPhrase().empty())
							cout << "   JoinPhrase: " << credit->JoinPhrase() << endl;
					}
				}
			}
		}
	}
	else
	{
		throw(runtime_error("Invalid CD record ID."));
	}
}

