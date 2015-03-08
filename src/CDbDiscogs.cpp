/**
 * Libraries: libmusicbrainz5, libdiscid
 */

#include "CDbDiscogs.h"

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "SCueSheet.h"
#include "credirect.h" // to redirect std::cerr stream

using std::cout;
using std::endl;

using std::ostringstream;
using std::deque;
using std::string;
using std::runtime_error;
using std::to_string;

/** Constructor.
 *
 *  @param[in] Client program name. If omitted or empty, uses "autorip"
 *  @param[in] Client program version. If omitted or empty, uses "alpha"
 */
CDbDiscogs::CDbDiscogs(const std::string &cname,const std::string &cversion)
    : CDbHttpBase(cname,cversion), CDbJsonBase()
{}

/** Constructor.
 *
 *  @param[in] Client program name. If omitted or empty, uses "autorip"
 *  @param[in] Client program version. If omitted or empty, uses "alpha"
 */
CDbDiscogs(const CDbMusicBrainz &mb, const std::string &cname,const std::string &cversion)
    : CDbHttpBase(cname,cversion), CDbJsonBase()
{
    // for each discogs release URL

}

CDbDiscogs::~CDbDiscogs() {}

/** Perform a new Discogs query given a list of its release IDs
 *
 *  @param[in] collection of release IDs
 *  @return    Number of valid records
 */
virtual int Query(const std::deque<std::string> &list)
{
    // if empty deque given, do nothing
    if (releases.empty()) return 0;

    ClearDiscs_();

    // look up all releases
    deque<string>::iterator it;
    for (it=list.begin();it!=list.end();it++)
    {
        Perform_(base_url+"releases/"+*it);

        // now load the data onto json object
        json_error_t error;
        json_t *root = json_loadb(data.c_str(), data.size(), 0, &error);
        if(!root) throw(std::runtime_error(error.text));

        // push the new release record onto the deque
        Releases.push_back(root);
    }

    // return the number of matches
    return Releases.size();
}

/** Get album title
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Title string (empty if title not available)
 */
std::string CDbDiscogs::AlbumTitle(const int recnum) const
{
    // set record
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid Release record ID."));

    return FindString_(Releases[recnum],"title");
}

/** Get album artist
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Artist string (empty if artist not available)
 */
std::string CDbDiscogs::AlbumArtist(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    CArtistCredit *credit = Releases[recnum].ArtistCredit();
    if (credit) return GetArtistString_(*credit);
    else return "";
}

/** Get label name
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Label string (empty if label not available)
 */
std::string CDbDiscogs::AlbumLabel(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    CLabelInfoList *list = Releases[recnum].LabelInfoList();
    CLabelInfo *info;
    CLabel *label;
    if (list && list->NumItems() && (info=list->Item(0)) && (label=info->Label())) return label->Name();
    else return "";
}

/** Get album UPC
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    UPC string (empty if title not available)
 */
std::string CDbDiscogs::AlbumUPC(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].Barcode();
}

/** Retrieve the disc info from specified database record
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    SDbrBase Pointer to newly created database record object. Caller is
 *             responsible for deleting the object.
 */
SDbrBase* CDbDiscogs::Retrieve(const int recnum) const
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
        rec->Rems.emplace_back("UPC ");
        rec->Rems[nrems].append(r.Barcode());
        nrems++;
    }

    if (!r.ASIN().empty())
    {
        rec->Rems.emplace_back("ASIN ");
        rec->Rems[nrems].append(r.ASIN());
        nrems++;
    }

    CLabelInfoList *list = r.LabelInfoList();
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
void CDbDiscogs::ClearDiscs_()
{
    // clear the list of matched releases and their coverart info
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
void CDbDiscogs::InitDisc_(const std::string &path)
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

void CDbDiscogs::Print(const int recnum) const
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
