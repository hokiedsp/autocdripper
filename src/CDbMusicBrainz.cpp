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

#include <coverart/HTTPFetch.h>
#include <coverart/ImageList.h>
#include <coverart/Image.h>
#include <coverart/Thumbnails.h>

#include <discid/discid.h>


using std::cout;
using std::endl;

using std::ostringstream;
using std::deque;
using std::string;
using std::runtime_error;
using std::to_string;

using namespace MusicBrainz5;
using namespace CoverArtArchive;

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
    MB5(cname+"-"+cversion,servername,serverport), CAA(cname+"-"+cversion), CoverArtSize(0)
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
        if (r->MediaMatchingDiscID (discid).NumItems())
        {
            Releases.emplace_back(*r);

            // also look for the coverart
            try
            {
                CoverArts.emplace_back(CAA.ReleaseInfo(r->ID()));
            }
            catch (CResourceNotFoundError err)
            {
                CoverArts.emplace_back(); // not found, enque empty info
            }
        }
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

/** Form an artist credit string 
 *
 *  @param[in] ArtistCredit query data
 *  @param[in] true to use SortName (if available) for the first asrtist's name
 *             (default is false)
 *  @return    Artist name in plain string
 */
std::string CDbMusicBrainz::GetArtistString_(const MusicBrainz5::CArtistCredit &credit, const bool sortfirst)
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
SDbrBase* CDbMusicBrainz::Retrieve(const int recnum)
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
    const CRelease &r = Releases[recnum];

    if (r.ArtistCredit ())
        rec->Performer = GetArtistString_(*r.ArtistCredit());
    rec->Title = r.Title();

    rec->Rems.emplace_back("DBSRC MusicBrainz");
    rec->Rems.emplace_back("DISCID ");	// comments on the disc
    rec->Rems[1].append(discid);

    rec->Rems.emplace_back("MBID ");
    rec->Rems[2].append(r.ID());
    nrems = 3;

    if (!r.Date().empty())
    {
        rec->Rems.emplace_back("DATE ");
        rec->Rems[3].append(r.Date());
        nrems++;
    }

    if (!r.Country().empty())
    {
        rec->Rems.emplace_back("COUNTRY ");
        rec->Rems[nrems].append(r.Country());
        nrems++;
    }

    if (!r.Barcode().empty())
    {
        rec->Rems.emplace_back("BARCODE ");
        rec->Rems[nrems].append(r.Barcode());
        nrems++;
    }

    if (!r.ASIN().empty())
    {
        rec->Rems.emplace_back("ASIN ");
        rec->Rems[nrems].append(r.ASIN());
        nrems++;
    }

    if (r.LabelInfoList() && r.LabelInfoList()->NumItems()>0)
    {
        CLabel *info = r.LabelInfoList ()->Item(0)->Label();
        if (!info->Name().empty())
        {
            rec->Rems.emplace_back("LABEL ");
            rec->Rems[nrems].append(info->Name());
            nrems++;
        }
    }

    // get cd media info
    CMediumList media = r.MediaMatchingDiscID(discid); // guarantees to return non-NULL
    CMedium *medium = media.Item(0);

    // get disc# and total # of cds if multiple-cd set
    if (r.MediumList()->NumItems()>1)
    {
        rec->Rems.emplace_back("DISC ");
        rec->Rems[nrems].append(to_string(medium->Position()));
        nrems++;

        rec->Rems.emplace_back("DISCS ");
        rec->Rems[nrems].append(to_string(r.MediumList()->NumItems()));
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

/** Clear all the disc entries
 */
void CDbMusicBrainz::ClearDiscs_()
{
    // clear the list of matched releases and their coverart info
    Releases.clear();
    CoverArts.clear();
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
void CDbMusicBrainz::FrontData(std::vector<unsigned char> &data, const int recnum) const
{
    if (recnum<0||recnum>(int)CoverArts.size())
        throw(runtime_error("Invalid Record Index requested."));

}

/** Check if the query returned a front cover
 *
 *  @param[out] image data buffer.
 *  @param[in]  record index (default=0)
 */
void CDbMusicBrainz::BackData(std::vector<unsigned char> &data, const int recnum) const
{
    if (recnum<0||recnum>(int)CoverArts.size())
        throw(runtime_error("Invalid Record Index requested."));

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
    if (recnum<0||recnum>(int)CoverArts.size())
        throw(runtime_error("Invalid Record Index requested."));

    const CImageList *info = CoverArts[recnum].ImageList();
    if (!info) return ""; // coverart not found

    for (int i=0;i<info->NumItems();i++)
    {
        const CImage *image = info->Item(i);
        if (image->Back())  // back cover found,look for request image size
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
