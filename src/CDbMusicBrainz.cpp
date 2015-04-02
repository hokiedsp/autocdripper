/**
 * Libraries: libmusicbrainz5, libdiscid
 */

#include "CDbMusicBrainz.h"

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "SCueSheet.h"
#include "credirect.h" // to redirect std::cerr stream

#include <musicbrainz5/Metadata.h>
#include <musicbrainz5/Disc.h>
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
#include <musicbrainz5/HTTPFetch.h>
#include <musicbrainz5/RelationListList.h>
#include <musicbrainz5/RelationList.h>
#include <musicbrainz5/Relation.h>

#include <musicbrainz5/TextRepresentation.h>
#include <musicbrainz5/ArtistCredit.h>
#include <musicbrainz5/RelationListList.h>
#include <musicbrainz5/Relation.h>

#include <coverart/HTTPFetch.h>
#include <coverart/ImageList.h>
#include <coverart/Image.h>
#include <coverart/Thumbnails.h>

#include <discid/discid.h>

using std::cout;
using std::endl;

using std::ostringstream;
using std::vector;
using std::string;
using std::runtime_error;
using std::to_string;

using namespace MusicBrainz5;
using namespace CoverArtArchive;

/** Constructor->l785
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
    MB5(cname+"-"+cversion,servername,serverport), CAA(cname+"-"+cversion), CoverArtSize(0)
{
    if (!username.empty()) MB5.SetUserName(username);
    if (!password.empty()) MB5.SetPassword(password);
}

CDbMusicBrainz::~CDbMusicBrainz()
{
    // Clear disc info collection
    Clear();
}

/** If AllowQueryCD() returns true, Query() performs a new query for the CD info
 *  in the specified drive with its *  tracks specified in the supplied cuesheet
 *  and its length. Previous query outcome discarded. After disc and its tracks
 *  are initialized, CDDB disc ID is computed. If the computation fails, function
 *  throws an runtime_error->l785l785
 *
 *  @param[in] CD-ROM device path
 *  @param[in] Cuesheet with its basic data populated
 *  @param[in] Length of the CD in sectors
 *  @param[in] (Optional) UPC barcode
 *  @return    Number of matched records
 */
int CDbMusicBrainz::Query(const std::string &dev, const SCueSheet &cuesheet, const size_t len, const std::string cdrom_upc)
{
    // redirect cerr stream during this function call
    cerr_redirect quiet_cerr;

    cout << "[CDbMusicBrainz::Query] 1\n";

    // must build disc based on cuesheet (throws error if fails to compute discid)
    InitDisc_(dev);

    cout << "[CDbMusicBrainz::Query] 2 discid: " << discid << endl;

    // Run the query
    try
    {
        CMetadata data = MB5.Query("discid", discid);

        CDisc *disc = data.Disc();
        CReleaseList *list;
        if (disc && (list=disc->ReleaseList()))
        {
            CRelease *r;
            CQuery::tParamMap Params;
            Params["inc"] = "artists labels recordings release-groups url-rels discids artist-credits";

            // Add all the matched releases to Releases deque
            MBQueries.reserve(list->NumItems());

            // Create new MBQueries element & populate
            for (int i=0; i<list->NumItems() && (r=list->Item(i)); i++)
            {
                MBQueries.emplace_back(); // create an empty instance
                CMetadata &rdata = MBQueries.back();
                try
                {
                    rdata = MB5.Query("release", r->ID(), "", Params);
                }
                catch (...)
                {
                    cout << "failed\n";
                    MBQueries.pop_back();
                    throw;
                }
                if (!rdata.Release()) MBQueries.pop_back();
            }

            // Populate Releases vector (safe as MBQueries is fixed until next query)
            Releases.reserve(MBQueries.size());
            for (vector<CMetadata>::iterator it = MBQueries.begin();
                 it != MBQueries.end();
                 it ++)
                Releases.push_back((*it).Release());

            // Populate CoverArts vector
            CoverArts.reserve(MBQueries.size());
            for (vector<CRelease*>::iterator it = Releases.begin();
                 it != Releases.end();
                 it ++)
            {
                // also look for the coverart
                CoverArts.emplace_back();
                try
                {
                    CoverArts.back() = (CAA.ReleaseInfo((*it)->ID()));
                }
                catch (...) {} //don't care if failed in any ways
            }
        }
    }
    catch (MusicBrainz5::CResourceNotFoundError &e) {} // no match found, just return

    // return the number of matches
    return Releases.size();
}

/** Look up full disc information from MusicBrainz server. It supports single record or
 *  multiple records if multiple entries were found by Query(). If the computation
 *  fails, function throws an runtime_error.
 *
 *  @param[in] Disc record ID (0-based index to discs). If negative, retrieves info
 *             for all records.
 *  @param[in] Network time out in seconds. CDbMusicBrainz does not use this parameter
 */
void CDbMusicBrainz::Populate(const int recnum)
{
}

    /** Clear all the disc entries
 */
void CDbMusicBrainz::Clear()
{
    // clear the list of matched releases and their coverart info
    MBQueries.clear();
    Releases.clear();
    CoverArts.clear();
    discid.clear();
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

/////////////////////////////////////////////////////////////////////////////////

/** Return a unique release ID string
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return id string if Query was successful.
 */
std::string CDbMusicBrainz::ReleaseId(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum]->ID();
}

/** Get album title
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Title string (empty if title not available)
 */
std::string CDbMusicBrainz::AlbumTitle(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum]->Title();
}

/** Get album artist
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Artist string (empty if artist not available)
 */
std::string CDbMusicBrainz::AlbumArtist(const int recnum) const
{
    string rval;

    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    CArtistCredit *credit = Releases[recnum]->ArtistCredit();
    if (credit) rval = GetArtistString_(*credit);

    return rval;
}

/** Get release date
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Date string (empty if genre not available)
 */
std::string CDbMusicBrainz::Date(const int recnum) const
{
    string rval;

    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum]->Date();
}

/** Get release country
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Countery string (empty if genre not available)
 */
std::string CDbMusicBrainz::Country(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum]->Country();
}

/**
 * @brief Get disc number
 * @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *            is returned.
 * @return    Disc number or -1 if unknown
 */
int CDbMusicBrainz::DiscNumber(const int recnum) const
{
    int rval = -1;

    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    // get cd media info
    CMediumList media = Releases[recnum]->MediaMatchingDiscID(discid); // guarantees to return non-NULL
    CMedium *medium = media.Item(0);

    // get disc# and total # of cds if multiple-cd set
    if (medium) rval = medium->Position();

    return rval;
}

/**
 * @brief Get total number of discs in the release
 * @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *            is returned.
 * @return    Number of discs or -1 if unknown
 */
int CDbMusicBrainz::TotalDiscs(const int recnum) const
{
    int rval = -1;

    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    // get disc# and total # of cds if multiple-cd set
    if (Releases[recnum]->MediumList())
        rval = Releases[recnum]->MediumList()->NumItems();

    return rval;
}

/** Get label name
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Label string (empty if label not available)
 */
std::string CDbMusicBrainz::AlbumLabel(const int recnum) const
{
    string rval;

    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    CLabelInfoList *list = Releases[recnum]->LabelInfoList();
    CLabelInfo *info;
    CLabel *label;
    if (list && list->NumItems() && (info=list->Item(0)) && (label=info->Label()))
        rval = label->Name();

    return rval;
}

/** Get album UPC
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    UPC string (empty if title not available)
 */
std::string CDbMusicBrainz::AlbumUPC(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum]->Barcode();
}


/** Get number of tracks
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    number of tracks
 *  @throw     runtime_error if CD record id is invalid
 */
int CDbMusicBrainz::NumberOfTracks(const int recnum) const
{
    int rval;

    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    // get cd media info
    CMediumList media = Releases[recnum]->MediaMatchingDiscID(discid); // guarantees to return non-NULL
    CMedium *medium = media.Item(0);

    // initialize tracks
    if (medium->TrackList())
        rval = medium->TrackList()->NumItems();
    else
        throw(runtime_error("Could not find the number of tracks."));

    return rval;
}

/** Get track title
 *
 *  @param[in] Track number (1-99)
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Title string (empty if title not available)
 *  @throw     runtime_error if track number is invalid
 */
std::string CDbMusicBrainz::TrackTitle(int tracknum, const int recnum) const
{
    string rval;

    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    // get cd media info
    CMediumList media = Releases[recnum]->MediaMatchingDiscID(discid); // guarantees to return non-NULL
    CMedium *medium = media.Item(0);

    // initialize tracks
    if (medium->TrackList())
    {
        int num_tracks = medium->TrackList()->NumItems();

        if (tracknum<1 || tracknum>num_tracks)
            throw(runtime_error("Invalid track number."));

        CTrack *track = medium->TrackList()->Item(--tracknum);
        if (!track)
            throw(runtime_error("Failed to retrieve track info."));

        CRecording *recording = track->Recording();
        if (!recording)
            throw(runtime_error("Failed to retrieve track recording info."));

        rval = recording->Title();
        if (rval.empty()) rval = track->Title();
    }

    return rval;
}

/** Get track artist
 *
 *  @param[in] Track number (1-99)
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Artist string (empty if artist not available)
 *  @throw     runtime_error if track number is invalid
 */
std::string CDbMusicBrainz::TrackArtist(int tracknum, const int recnum) const
{
    string rval;

    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    // get cd media info
    CMediumList media = Releases[recnum]->MediaMatchingDiscID(discid); // guarantees to return non-NULL
    CMedium *medium = media.Item(0);

    // initialize tracks
    if (medium->TrackList())
    {
        int num_tracks = medium->TrackList()->NumItems();

        if (tracknum<1 || tracknum>num_tracks)
            throw(runtime_error("Invalid track number."));

        CTrack *track = medium->TrackList()->Item(--tracknum);
        if (!track)
            throw(runtime_error("Failed to retrieve track info."));

        CRecording *recording = track->Recording();
        if (!recording)
            throw(runtime_error("Failed to retrieve track recording info."));

        if (recording && recording->ArtistCredit())
            rval = GetArtistString_(*recording->ArtistCredit());
        if (rval.empty() && track->ArtistCredit())
            rval = GetArtistString_(*track->ArtistCredit());
    }

    return rval;
}

/** Get track ISRC
 *
 *  @param[in] Track number (1-99)
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    ISRC string
 *  @throw     runtime_error if track number is invalid
 */
std::string CDbMusicBrainz::TrackISRC(int tracknum, const int recnum) const
{
    string rval;

    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    // get cd media info
    CMediumList media = Releases[recnum]->MediaMatchingDiscID(discid); // guarantees to return non-NULL
    CMedium *medium = media.Item(0);

    // initialize tracks
    if (medium->TrackList())
    {
        int num_tracks = medium->TrackList()->NumItems();

        if (tracknum<1 || tracknum>num_tracks)
            throw(runtime_error("Invalid track number."));

        CTrack *track = medium->TrackList()->Item(--tracknum);
        if (!track)
            throw(runtime_error("Failed to retrieve track info."));

        CRecording *recording = track->Recording();
        if (!recording)
            throw(runtime_error("Failed to retrieve track recording info."));

        CISRCList *isrcs = recording->ISRCList();
        if (isrcs && isrcs->NumItems()>0)
            rval = isrcs->Item(0)->ID();
    }

    return rval;
}



/** Form an artist credit string 
 *
 *  @param[in] ArtistCredit query data
 *  @param[in] true to use SortName (if available) for the first asrtist's name
 *             (default is false)
 *  @return    Artist name in plain string
 */
std::string CDbMusicBrainz::GetArtistString_(const MusicBrainz5::CArtistCredit &credit, const bool sortfirst) const
{
    string str;
    CNameCredit *name;
    CArtist *artist;
    int n;

    // get the name list
    CNameCreditList * list = credit.NameCreditList();
    if (!(list && (n=list->NumItems()))) return str; // no credit

    // get the first artist
    name = list->Item(0);
    if (!name) return str;

    artist=name->Artist();
    if (artist)
    {
        // grab the first artist's name
        if (sortfirst) str = artist->SortName();
        if (!sortfirst || str.empty()) str = artist->Name();
        if (str.empty()) str = name->Name();
    }

    // append join phrase if available
    str.append(name->JoinPhrase());

    for (int i=1;i<n;i++)
    {
        name = list->Item(i);
        if (!name) return str; // no name, abort
        artist=name->Artist();

        string next;
        if (artist)
        {
            // grab the first artist's name
            next = artist->Name();
            if (next.empty()) str = name->Name();
        }

        str.append(next);
        str.append(name->JoinPhrase());
    }

    return str;
}

/** Retrieve the disc info from specified database record
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    SDbrBase Pointer to newly created database record object. Caller is
 *             responsible for deleting the object.
 */
SDbrBase* CDbMusicBrainz::Retrieve(const int recnum) const
{
    int nrems;
    //const char *str;	// temp
    //ostringstream stream; // for formatting

    // instantiate new DBR object
    SDbrMusicBrainz * rec = new SDbrMusicBrainz;

    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    // Grab the specified release info
    const CRelease *r = Releases[recnum];

    if (r->ArtistCredit ())
        rec->Performer = GetArtistString_(*r->ArtistCredit());
    rec->Title = r->Title();

    rec->Rems.emplace_back("DBSRC MusicBrainz");
    rec->Rems.emplace_back("DISCID ");	// comments on the disc
    rec->Rems[1].append(discid);

    rec->Rems.emplace_back("MBID ");
    rec->Rems[2].append(r->ID());
    nrems = 3;

    if (!r->Date().empty())
    {
        rec->Rems.emplace_back("DATE ");
        rec->Rems[3].append(r->Date());
        nrems++;
    }

    if (!r->Country().empty())
    {
        rec->Rems.emplace_back("COUNTRY ");
        rec->Rems[nrems].append(r->Country());
        nrems++;
    }

    if (!r->Barcode().empty())
    {
        rec->Rems.emplace_back("UPC ");
        rec->Rems[nrems].append(r->Barcode());
        nrems++;
    }

    if (!r->ASIN().empty())
    {
        rec->Rems.emplace_back("ASIN ");
        rec->Rems[nrems].append(r->ASIN());
        nrems++;
    }

    CLabelInfoList *list = r->LabelInfoList();
    CLabelInfo *info;
    CLabel *label;
    string name;
    if (list && list->NumItems() && (info=list->Item(0)) && (label=info->Label()) && (name=label->Name()).size())
    {
        rec->Rems.emplace_back("LABEL ");
        rec->Rems[nrems].append(name);
        nrems++;
    }

    // get cd media info
    CMediumList media = r->MediaMatchingDiscID(discid); // guarantees to return non-NULL
    CMedium *medium = media.Item(0);

    // get disc# and total # of cds if multiple-cd set
    if (r->MediumList()->NumItems()>1)
    {
        rec->Rems.emplace_back("DISC ");
        rec->Rems[nrems].append(to_string(medium->Position()));
        nrems++;

        rec->Rems.emplace_back("DISCS ");
        rec->Rems[nrems].append(to_string(r->MediumList()->NumItems()));
        nrems++;
    }

    // initialize tracks
    if (!medium->TrackList()) return rec;
    int num_tracks = medium->TrackList()->NumItems();
    rec->AddTracks(num_tracks); // adds Tracks 1 to num_tracks

    string str;
    for (int i=0;i<num_tracks;i++)
    {
        CTrack *track = medium->TrackList()->Item(i);
        if (!track) continue;
        CRecording *recording = track->Recording();

        // get the track object
        SCueTrack &rectrack = rec->Tracks[i];

        if (recording)	str = recording->Title();
        else str.clear();
        if (str.empty()) str = track->Title();
        rectrack.Title = str;

        if (recording && recording->ArtistCredit())
            str = GetArtistString_(*recording->ArtistCredit());
        else
            str.clear();
        if (str.empty() && track->ArtistCredit())
            str = GetArtistString_(*track->ArtistCredit());
        rectrack.Performer = str;

        CISRCList *isrcs = recording->ISRCList();
        if (isrcs && isrcs->NumItems()>0)
            rectrack.ISRC = isrcs->Item(0)->ID();
    }

    return rec;
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
    Clear();

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
        vector<CRelease*>::const_iterator it;
        for (it=Releases.begin(); it!=Releases.end(); it++)
        {
            const CRelease *r = *it;

            // Retrieve and print the category and disc ID.
            cout << r->Title() << " (";

            // if multi-disc release, insert the disc position
            if (r->MediumList ()->NumItems()>1)
                cout << r->MediaMatchingDiscID(discid).Item(0)->Position()
                     << " out of " << r->MediumList ()->NumItems() << ") (";

            if (r->Date().empty()) cout << "Unknown";
            else cout << r->Date ();
            cout << ":";
            if (!r->TextRepresentation() || r->TextRepresentation()->Language().empty()) cout << "UNKNOWN";
            else cout << r->TextRepresentation ()->Language();
            cout << ")" << endl;
        }
    }
    else if (recnum<(int)Releases.size())	// single disc
    {
        const CRelease *r = Releases[recnum];

        cout << " ID: " << r->ID() << endl;
        cout << " Title: " << r->Title() << endl;
        cout << " Status: " << r->Status () << endl;
        cout << " Quality: " << r->Quality () << endl;
        cout << " Disambiguation: " << r->Disambiguation () << endl;
        cout << " Packaging: " << r->Packaging () << endl;
        cout << " TextRepresentation/Language: " << r->TextRepresentation ()->Language() << endl;
        cout << " TextRepresentation/Script: " << r->TextRepresentation ()->Script() << endl;
        if (r->ArtistCredit ())
        {
            CNameCreditList * list = r->ArtistCredit()->NameCreditList ();
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
        if (r->ReleaseGroup ())
        {
            cout << " Release Group ID: " << r->ReleaseGroup ()->ID() << endl;
            cout << "  Primary Type: " << r->ReleaseGroup()->PrimaryType () << endl;
            cout << "  Title: " << r->ReleaseGroup()->Title () << endl;
            cout << "  Disambiguation: " << r->ReleaseGroup()->Disambiguation () << endl;
            cout << "  First Release Date: " << r->ReleaseGroup()->FirstReleaseDate () << endl;
        }
        cout << " Date: " << r->Date () << endl;
        cout << " Country: " << r->Country () << endl;
        cout << " Barcode: " << r->Barcode () << endl;
        cout << " ASIN: " << r->ASIN () << endl;

        if (r->LabelInfoList ())
        {
            for (int i=0;i<r->LabelInfoList ()->NumItems();i++)
            {
                CLabel *info = r->LabelInfoList ()->Item(i)->Label();
                cout << " Label " << i << ": " << info->ID () << endl;
                cout << "  Type " << info->Type () << endl;
                cout << "  Name " << info->Name () << endl;
                cout << "  Sort Name " << info->SortName () << endl;
                cout << "  Label Code " << info->LabelCode () << endl;
                cout << "  Disambiguation " << info->Disambiguation () << endl;
                cout << "  Country " << info->Country () << endl;
            }
        }

        if (r->MediumList ())
            cout << " MediumList Items: " << r->MediumList ()->NumItems() << endl; // total discs

        if (r->RelationListList()) // link to amazon or discog
        {
            for (int i = 0;i<r->RelationListList ()->NumItems(); i++)
            {
                CRelationList *list = r->RelationListList ()->Item(i);
                cout << " Relation List" << i << ": " << list->TargetType()  << endl;
                for (int j = 0;j<list->NumItems();j++)
                {
                    CRelation *rel = list->Item(j);
                    cout << "  Relation " << j << ": " << rel->Type() << endl;
                    cout << "   Target: " << rel->Target() << endl;
                }
            }
        }

        if (r->CollectionList())
            cout << " CollectionList Items: " << r->CollectionList ()->NumItems() << endl;
        cout << endl;

        CMediumList media = r->MediaMatchingDiscID(discid);
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


/** Specify the preferred coverart image width
 *
 *  @param[in] Preferred width of the image
 */
void CDbMusicBrainz::SetPreferredWidth(const size_t &width)
{
    if (width>500) CoverArtSize = 0;
    else if (width>250) CoverArtSize = 1;
    else CoverArtSize = 2;
}

/** Specify the preferred coverart image height
 *
 *  @param[in] Preferred height of the image
 */
void CDbMusicBrainz::SetPreferredHeight(const size_t &height)
{
    if (height>500) CoverArtSize = 0;
    else if (height>250) CoverArtSize = 1;
    else CoverArtSize = 2;
}


/** Check if the query returned a front cover
 *
 *  @param[in]  record index (default=0)
 *  @return     true if front cover is found.
 */
bool CDbMusicBrainz::Front(const int recnum) const
{
    if (recnum<0||recnum>(int)CoverArts.size())
        throw(runtime_error("Invalid Record Index requested."));

    const CImageList *info = CoverArts[recnum].ImageList();
    if (!info) return false; // coverart not found

    for (int i=0;i<info->NumItems();i++)
    {
        if (info->Item(i)->Front()) return true;
    }
    return false;
}

/** Check if the query returned a back cover
 *
 *  @param[in]  record index (default=0)
 *  @return     true if back cover is found.
 */
bool CDbMusicBrainz::Back(const int recnum) const
{
    if (recnum<0||recnum>(int)CoverArts.size())
        throw(runtime_error("Invalid Record Index requested."));

    const CImageList *info = CoverArts[recnum].ImageList();
    if (!info) return false; // coverart not found

    for (int i=0;i<info->NumItems();i++)
    {
        if (info->Item(i)->Back()) return true;
    }
    return false;
}

/** Retrieve the front cover data.
 *
 *  @param[out] image data buffer.
 *  @param[in]  record index (default=0)
 */
std::vector<unsigned char> CDbMusicBrainz::FrontData(const int recnum) const
{
    if (recnum<0||recnum>(int)CoverArts.size())
        throw(runtime_error("Invalid Record Index requested."));

    return CAA.FetchFront(Releases[recnum]->ID());
}

/** Check if the query returned a front cover
 *
 *  @param[out] image data buffer.
 *  @param[in]  record index (default=0)
 */
std::vector<unsigned char> CDbMusicBrainz::BackData(const int recnum) const
{
    if (recnum<0||recnum>(int)CoverArts.size())
        throw(runtime_error("Invalid Record Index requested."));

    return CAA.FetchBack(Releases[recnum]->ID());
}

/** Get the URL of the front cover image
 *
 *  @param[in]  Record index (default=0)
 *  @return     URL string
 */
std::string CDbMusicBrainz::FrontURL(const int recnum) const
{
    if (recnum<0||recnum>(int)CoverArts.size())
        throw(runtime_error("Invalid Record Index requested."));

    const CImageList *info = CoverArts[recnum].ImageList();
    if (!info) return ""; // coverart not found

    for (int i=0;i<info->NumItems();i++)
    {
        const CImage *image = info->Item(i);
        if (image->Front())  // front cover found,look for request image size
        {
            string url;
            const CThumbnails *tn = image->Thumbnails();
            if (CoverArtSize==2 && !(url=tn->Small()).empty()) return url;
            else if (CoverArtSize>0 && !(url=tn->Large()).empty()) return url;
            else return image->Image();
        }
    }
    return "";
}

/** Get the URL of the back cover image
 *
 *  @param[in]  Record index (default=0)
 *  @return     URL string
 */
std::string CDbMusicBrainz::BackURL(const int recnum) const
{
    string rval;

    if (recnum<0||recnum>(int)CoverArts.size())
        throw(runtime_error("Invalid Record Index requested."));

    const CImageList *info = CoverArts[recnum].ImageList();
    if (info)// only if coverart found
    {
        for (int i=0;i<info->NumItems()&&rval.size();i++)
        {
            const CImage *image = info->Item(i);
            if (image->Back())  // back cover found,look for request image size
            {
                string url;
                const CThumbnails *tn = image->Thumbnails();
                if (CoverArtSize==2 && !(url=tn->Small()).empty()) rval = url;
                else if (CoverArtSize>0 && !(url=tn->Large()).empty()) rval = url;
                else rval = image->Image();
            }
        }
    }
    return "";
}

/**
 * @brief Get a related URL.
 * @param[in]  Relationship type (e.g., "discogs", "amazon asin")
 * @param[in]  Record index (default=0)
 * @return URL string or empty if requestd URL type not in the URL
 */
std::string CDbMusicBrainz::RelationUrl(const std::string &type, const int recnum)
{
    // redirect cerr stream during this function call
    cerr_redirect quiet_cerr;

    string rval;
    MusicBrainz5::CRelationListList *relslist = NULL;
    CMetadata data;

    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    // First check the RelationListList in the Release
    relslist = Releases[recnum]->RelationListList();
    if (!relslist)
    {
        CQuery::tParamMap params;
        params["inc"] = "url-rels";
        try
        {
            data = MB5.Query("release", Releases[recnum]->ID(), "", params);
            relslist = data.Release()->RelationListList();
        }
        catch (MusicBrainz5::CResourceNotFoundError &e) {} // no match found, just return
    }
    if (relslist) rval = MB5RelationUrl_(relslist,type);

    if (rval.empty()) // if none found in Release, check ReleaseGroup
    {
        CReleaseGroup *rg = Releases[recnum]->ReleaseGroup();
        if (rg)
        {
            relslist = rg->RelationListList();
            if (!relslist)
            {
                CQuery::tParamMap params;
                params["inc"] = "url-rels";
                try
                {
                    data = MB5.Query("release-group", rg->ID(), "", params);

                    // if releasegroup returned, get its relation lists
                    if ((rg = data.ReleaseGroup()))
                        relslist = rg->RelationListList();
                }
                catch (MusicBrainz5::CResourceNotFoundError &e) {} // just continue
            }

            if (relslist) rval = MB5RelationUrl_(relslist, type);
        }
    }

    return rval;
}

/**
 * @brief Helper function to RelationUrl
 * @param A pointer to MB5 meta (must be non-null)
 * @param[in]  Relationship type (e.g., "discogs", "amazon asin")
 * @return URL string or empty if requestd URL type not in the URL
 */
std::string CDbMusicBrainz::MB5RelationUrl_(MusicBrainz5::CRelationListList *relslist,
                                            const std::string &type) const
{
    string rval;

    MusicBrainz5::CRelationList *urls = NULL;
    for (int i = 0; !urls && i<relslist->NumItems(); i++)   // look for URL list
    {
        MusicBrainz5::CRelationList *rels = relslist->Item(i);
        if (rels && rels->TargetType().compare("url")==0)
        {
            urls = rels;
            for (int j = 0; rval.empty() && j<urls->NumItems(); j++)
            {
                MusicBrainz5::CRelation *url = urls->Item(j);
                if (url->Type().compare(type)==0) rval = url->Target();
            }
        }
    }

    return rval;
}
