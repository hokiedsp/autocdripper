/**
 * Libraries: libmusicbrainz5, libdiscid
 */

#include "CDbLastFm.h"

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstdint>

#include "SCueSheet.h"
#include "credirect.h" // to redirect std::cerr stream

using std::cout;
using std::endl;

using std::ostringstream;
using std::deque;
using std::string;
using std::runtime_error;
using std::to_string;

const std::string CDbLastFm::base_url("http://ws.audioscrobbler.com/2.0/");

/** Constructor.
 *
 *  @param[in] Client program last.fm API key
 *  @param[in] Client program name. If omitted or empty, uses "autorip"
 *  @param[in] Client program version. If omitted or empty, uses "alpha"
 */
CDbLastFm::CDbLastFm(const std::string &key, const std::string &cname,const std::string &cversion)
    : CDbHttpBase(cname,cversion), CDbJsonBase(), apikey(key), CoverArtSize(3)
{}

/** Constructor.
 *
 *  @param[in] Client program last.fm API key
 *  @param[in] Client program name. If omitted or empty, uses "autorip"
 *  @param[in] Client program version. If omitted or empty, uses "alpha"
 */
CDbLastFm::CDbLastFm(const CDbMusicBrainz &mb, const std::string &key, const std::string &cname,const std::string &cversion)
    : CDbHttpBase(cname,cversion), CDbJsonBase(), apikey(key), CoverArtSize(3)
{
    // for each LastFm release URL

}

CDbLastFm::~CDbLastFm() {}

/** Perform a new LastFm query given a list of MusicBrainz release IDs
 *
 *  @param[in] collection of MusicBrainz release IDs
 *  @return    Number of valid records
 */
int CDbLastFm::Query(const std::deque<std::string> &list)
{
    // if empty deque given, do nothing
    if (list.empty()) return 0;

    ClearReleases_();

    // look up all releases
    deque<string>::const_iterator it;
    for (it=list.begin();it!=list.end();it++)
    {
        PerformHttpTransfer_(FormUrlFromMbid(*it, apikey));
        AppendRelease_(data); // Create a new JSON object
        if (!FindObject_(Releases.back(),"album",Releases.back()))
            throw(runtime_error("Received album data is corrupted."));
    }

    // return the number of matches
    return Releases.size();
}

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
int CDbLastFm::Search(const std::string &title, const std::string &artist, const bool autofill, const int timeout)
{
    return 0;
}

/** Returns the CD record ID associated with the specified genre. If no matching
 *  record is found, it returns -1.
 *
 *  @return Matching CD record ID.
 */
int CDbLastFm::MatchByGenre(const std::string &genre) const
{
    return -1;
}

/** Get album title
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Title string (empty if title not available)
 */
std::string CDbLastFm::AlbumTitle(const int recnum) const
{
    // set record
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid Release record ID."));

    return Title_(Releases[recnum]);
}

std::string CDbLastFm::Title_(const json_t* json)
{
    string titlestr;
    if (FindString_(json,"name",titlestr)) return titlestr;
    return "";
}

/** Get album artist
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Artist string (empty if artist not available)
 */
std::string CDbLastFm::AlbumArtist(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return AlbumArtist_(Releases[recnum]);
}

/** Get genre
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    Empty string (LastFm does not support genre)
 */
std::string CDbLastFm::Genre(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return Genre_(Releases[recnum]);
}

std::string CDbLastFm::Genre_(const json_t* release)
{
    json_t *toptags, *tag;
    string str;

    if (FindObject_(release,"toptags",toptags)
            && FindArray_(toptags,"tag",tag)
            && FindString_(json_array_get(tag,0),"name", str)) return str;
    else return "";
}

/**
 * @brief Returns number of tracks on the CD
 * @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 * @return Number of tracks
 */
int CDbLastFm::NumberOfTracks(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return json_array_size(TrackList_(Releases[recnum]));
}

std::string CDbLastFm::AlbumArtist_(const json_t* json)
{
    string str;

    // Look for artist entries
    if (FindString_(json,"artist",str)) return str;
    else return "";
}

std::string CDbLastFm::Date_(const json_t* release)
{
    string str;
    if (FindString_(release,"releasedate",str)) return str;
    else return "";
}

json_t* CDbLastFm::TrackList_(const json_t *release)
{
    json_t *tracks, *track;

    if (FindObject_(release,"tracks",tracks) && FindArray_(tracks,"track",track))
        return track;
    else
        throw(runtime_error("Tracklist not found."));
}

/** Retrieve the disc info from specified database record
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return    SDbrBase Pointer to newly created database record object. Caller is
 *             responsible for deleting the object.
 */
SDbrBase* CDbLastFm::Retrieve(const int recnum) const
{
    json_t *tracks;
    int num_tracks;
    string str;

    // instantiate new DBR object
    SDbrLastFm * rec = new SDbrLastFm;

    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid Release record ID."));

    // Grab the specified release info
    const json_t *r = Releases[recnum];

    rec->Title = Title_(r);
    rec->Performer = AlbumArtist_(r);

    rec->Rems.emplace_back("DBSRC LastFm");
    if (FindString_(r,"id",str))
        rec->Rems.emplace_back("ID "+str);	// comments on the disc

    str = Genre_(r);
    if (str.size()) rec->Rems.emplace_back("GENRE "+str);

    str = Date_(r);
    if (str.size())
        rec->Rems.emplace_back("DATE "+str);

    // initialize tracks
    tracks = TrackList_(r);
    num_tracks = json_array_size(tracks);
    rec->AddTracks(num_tracks); // adds Tracks 1 to num_tracks

    for (int i=0;i<num_tracks;i++)
    {
        json_t *track = json_array_get(tracks,i);

        // get the track object
        SCueTrack &rectrack = rec->Tracks[i];

        rectrack.Title = Title_(track);
        rectrack.Performer = TrackArtist_(track);

        //CISRCList *isrcs = recording->ISRCList();
        //if (isrcs && isrcs->NumItems()>0)
        //    rectrack.ISRC = isrcs->Item(0)->ID();
    }

    return rec;
}

std::string CDbLastFm::TrackArtist_(const json_t* data)
{
    json_t *artist;
    string str;
    if (FindObject_(data,"artist",artist) && FindString_(artist,"name",str))
        return str;
    else
        return "";
}

/**
 * @brief Form URL from MusicBrainz release ID
 * @param[in] MusicBrainz release ID
 * @return Generated URL
 */
std::string CDbLastFm::FormUrlFromMbid(const std::string &MBID, const std::string &apikey)
{
    string url(base_url);

    url.reserve(256);

    url += "?method=album.getinfo&format=json&api_key=";
    url += apikey;
    url += "&mbid=";
    url += MBID;

    return url;
}

/** Specify the preferred coverart image width
 *
 *  @param[in] Preferred width of the image
 */
void CDbLastFm::SetPreferredWidth(const size_t &width)
{
    if (width<=34) CoverArtSize = 0; // small
    else if (width<=64) CoverArtSize = 1; // medium
    else if (width<=126) CoverArtSize = 2; // large
    else if (width<=252) CoverArtSize = 3; // extralarge
    else CoverArtSize = 4; // mega
}

/** Specify the preferred coverart image height
 *
 *  @param[in] Preferred height of the image
 */
void CDbLastFm::SetPreferredHeight(const size_t &height)
{
    if (height<=34) CoverArtSize = 0; // small
    else if (height<=64) CoverArtSize = 1; // medium
    else if (height<=126) CoverArtSize = 2; // large
    else if (height<=252) CoverArtSize = 3; // extralarge
    else CoverArtSize = 4; // mega
}

/** Check if the query returned a front cover
 *
 *  @param[in]  record index (default=0)
 *  @return     true if front cover is found.
 */
bool CDbLastFm::Front(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return ImageURL_(Releases[recnum], CoverArtSize).size();
}

/** Retrieve the front cover data.
 *
 *  @param[out] image data buffer.
 *  @param[in]  record index (default=0)
 */
std::vector<unsigned char> CDbLastFm::FrontData(const int recnum)
{
    std::vector<unsigned char> imdata;
    string url;
    size_t size;

    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    // get the URL of the image file
    url = ImageURL_(Releases[recnum], CoverArtSize);

    // get the image file size
    size = GetHttpContentLength_(url);
    if (size>0) imdata.reserve(size);

    // set imdata as the download buffer
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CDbHttpBase::write_uchar_vector_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &imdata);

    // perform the HTTP transaction
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK) throw(std::runtime_error(curl_easy_strerror(res)));

    cout << "IMAGE SIZE: " << imdata.size() << " bytes" << endl;

    // reset the download buffer
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CDbHttpBase::write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

    return imdata;
}

/** Get the URL of the front cover image
 *
 *  @param[in]  Record index (default=0)
 *  @return     URL string
 */
std::string CDbLastFm::FrontURL(const int recnum) const
{
    // set disc
    if (recnum<0 || recnum>=(int)Releases.size()) // all discs
        throw(runtime_error("Invalid CD record ID."));

    return ImageURL_(Releases[recnum], CoverArtSize);
}

std::string CDbLastFm::ImageURL_(const json_t *release, const int size)
{
    // Image sizes are assumed to be according to:
    // http://www.last.fm/group/Last.fm+Web+Services/forum/21604/_/544605/1#f9834498
    //
    //<image size="small" >…34px…</image>
    //<image size="medium">…64px…</image>
    //<image size="large">…126px…</image>
    //<image size="extralarge">…252px…</image>
    //<image size="mega">…500px…</image>

    json_t *imarray, *imobject;
    string url, sizestr;
    size_t index;

    // must have image array object
    if (!FindArray_(release,"image",imarray)) return "";

    // prepare size string
    switch (size)
    {
    case 0: sizestr = "small"; break;
    case 1: sizestr = "medium"; break;
    case 2: sizestr = "large"; break;
    case 3: sizestr = "extralarge"; break;
    default: sizestr = "mega";
    }

    // go through each image object (assume image array contains its immages in increasing size)
    json_array_foreach(imarray, index, imobject)
    {
        // grab the URL
        if (!FindString_(imobject,"#text",url)) continue;

        // check the image type
        if (CompareString_(imobject,"size",sizestr)==0) break;
    }

    return url;
}
