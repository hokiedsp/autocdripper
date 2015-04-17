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
#include "CDbMusicBrainz.h"
#include "CDbLastFmElem.h"

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
    : CUtilUrl(cname,cversion), apikey(key), CoverArtSize(3)
{}

CDbLastFm::~CDbLastFm() {}

/**
 * @brief Clear all the matches from previous search
 */
void CDbLastFm::Clear()
{
    Releases.clear();
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
    for (int i=0; i<mbdb.NumberOfMatches(); i++)
    {
        std::string mbid = mbdb.ReleaseId(i);
        if (mbid.size())
        {
            // form search URL
            string url(base_url);
            url.reserve(256);
            url += "?method=album.getinfo&format=json&api_key=";
            url += apikey;
            url += "&mbid=";
            url += mbid;

            // look up all releases
            PerformHttpTransfer_(url);

            // parse the downloaded JSON data
            CDbLastFmElem release(rawdata);

            // if images are available, keep the record
            if (release.HasImage())
            {
                Releases.emplace_back("");
                Releases.back().Swap(release);
            }
        }
    }

    cout << "[LastFM::Query] found " << Releases.size() << " releases with images\n";

    // return the number of matches
    return Releases.size();
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


/** Returns the number of matched records returned from the last Query() call.
 *
 *  @return    Number of matched records
 */
int CDbLastFm::NumberOfMatches() const
{
    return Releases.size();
}

/** Return the ID of the release
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return ID string
 */
std::string CDbLastFm::ReleaseId(const int recnum) const
{
    // check for the valid disc request
    if (Releases.empty() || recnum<0 || recnum>=(int)Releases.size())
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].ReleaseId();
}

/** Check if the query returned a front cover
 *
 *  @param[in]  record index (default=0)
 *  @return     true if front cover is found.
 */
bool CDbLastFm::Front(const int recnum) const
{
    // check for the valid disc request
    if (Releases.empty() || recnum<0 || recnum>=(int)Releases.size())
        throw(runtime_error("Invalid CD record ID."));

    return Releases[recnum].HasImage();
}

/** Get the URL of the front cover image
 *
 *  @param[in]  Record index (default=0)
 *  @return     URL string
 */
std::string CDbLastFm::FrontURL(const int recnum) const
{
    // check for the valid disc request
    if (Releases.empty() || recnum<0 || recnum>=(int)Releases.size())
        throw(runtime_error("Invalid CD record ID."));

    // get the URL of the image file
    return Releases[recnum].ImageURL(CoverArtSize);
}

/** Retrieve the front cover data.
 *
 *  @param[out] image data buffer.
 *  @param[in]  record index (default=0)
 */
UByteVector CDbLastFm::FrontData(const int recnum)
{
    // get the URL of the image file
    return DataToMemory(FrontURL(recnum));
}
