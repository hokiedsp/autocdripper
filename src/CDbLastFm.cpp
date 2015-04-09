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
#include "CDbMusicBrainz.h"

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
    : CDbHttpBase(cname,cversion), apikey(key), CoverArtSize(3)
{}

CDbLastFm::~CDbLastFm() {}

/**
 * @brief Clear all the matches from previous search
 */
void CDbLastFm::Clear()
{
    ClearData();
}

/** If MayBeLinkedFromMusicBrainz() returns true, Query() performs a new
 *  query based on the MusicBrainz query results.
 *
 *  @param[in] MusicBrainz database object.
 *  @param[in] (Optional) UPC barcode
 *  @return    Number of matched records
 */
int CDbLastFm::Query(CDbMusicBrainz &mbdb, const std::string upc)
{
    cout << "[LastFM::Query] starting\n";

    // clear the data
    Clear();

    // Get the MBID
    discid = mbdb.GetDiscId();
    if (discid.size())
    {
        // form search URL
        string url(base_url);
        url.reserve(256);
        url += "?method=album.getinfo&format=json&api_key=";
        url += apikey;
        url += "&mbid=";
        url += discid;

        // look up all releases
        PerformHttpTransfer_(url);

        std::cout << rawdata << std::endl;

        // parse the downloaded JSON data
        LoadData(rawdata);

        // donno what to expect, try
        PrintJSON();
    }

    // return the number of matches
    return data!=NULL;
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
    // check for the valid disc request
    if (!data || recnum!=0)
        throw(runtime_error("Invalid CD record ID."));

    return ImageURL_().size();
}

/** Retrieve the front cover data.
 *
 *  @param[out] image data buffer.
 *  @param[in]  record index (default=0)
 */
UByteVector CDbLastFm::FrontData(const int recnum) const
{
    UByteVector imdata;
    string url;
    size_t size;

    // check for the valid disc request
    if (data==NULL || recnum!=0)
        throw(runtime_error("Invalid CD record ID."));

    // get the URL of the image file
    url = ImageURL_();

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
    // check for the valid disc request
    if (data==NULL || recnum!=0)
        throw(runtime_error("Invalid CD record ID."));

    return ImageURL_();
}

std::string CDbLastFm::ImageURL_() const
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
    if (!FindArray("image",imarray)) return "";

    // prepare size string
    switch (CoverArtSize)
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
        if (!FindString(imobject,"#text",url)) continue;

        // check the image type
        if (CompareString(imobject,"size",sizestr)==0) break;
    }

    return url;
}
