#include "CDbAmazon.h"

#include <stdexcept>

#include "CDbMusicBrainz.h"
#include "CDbAmazonElem.h"

const std::string CDbAmazon::base_url = "http://lon.gr/ata/";

/**
 * @brief CDbAmazon
 * @param cname
 * @param cversion
 */
CDbAmazon::CDbAmazon(const std::string &asin, const std::string &cname,const std::string &cversion)
    : CUtilUrl(cname,cversion), CoverArtSize(2)
{
    // if ASIN is given, immediately perform a single item query
    if (asin.size()) Query(asin);
}

/**
 * @brief Clear all the matches from previous search
 */
void CDbAmazon::Clear()
{
    Releases.clear();
}


/** Single item query with ASIN
 *
 *  @param[in] Amazon Standard Identification Number
 *  @return    1 if item found or 0 if not
 */
int CDbAmazon::Query(const std::string asin)
{
    std::string url;
    url.reserve(256);

    // clear the data
    Clear();

    if (asin.size())
    {
        url = base_url + asin;

        // look up all releases
        PerformHttpTransfer_(url);

        // parse the downloaded JSON data
        CDbAmazonElem release(asin, rawdata);

        // if images are available, keep the record
        if (release.HasImage())
        {
            Releases.emplace_back("","");
            Releases.back().Swap(release);
        }
    }

    // return the number of matches
    return Releases.size();
}

/** If MayBeLinkedFromMusicBrainz() returns true, Query() performs a new
 *  query based on the MusicBrainz query results.
 *
 *  @param[in] MusicBrainz database object.
 *  @param[in] (Optional) UPC barcode
 *  @return    Number of matched records
 */
int CDbAmazon::Query(CDbMusicBrainz &mbdb, const std::string upc)
{
    std::string url, asin;
    url.reserve(256);

    // clear the data
    Clear();

    // Get the MBID
    for (int i=0; i<mbdb.NumberOfMatches(); i++)
    {
        asin = mbdb.AlbumASIN(i);
        if (asin.size())
        {
            url = base_url + asin;

            // look up all releases
            PerformHttpTransfer_(url);

            // parse the downloaded JSON data
            CDbAmazonElem release(asin, rawdata);

            // if images are available, keep the record
            if (release.HasImage())
            {
                Releases.emplace_back("","");
                Releases.back().Swap(release);
            }
        }
    }

    // return the number of matches
    return Releases.size();
}

/** Specify the preferred coverart image width
 *
 *  @param[in] Preferred width of the image
 */
void CDbAmazon::SetPreferredWidth(const size_t &width)
{
    if (width<=75) CoverArtSize = 0; // small
    else if (width<=160) CoverArtSize = 1; // medium
    else CoverArtSize = 2; // large
}

/** Specify the preferred coverart image height
 *
 *  @param[in] Preferred height of the image
 */
void CDbAmazon::SetPreferredHeight(const size_t &height)
{
    if (height<=75) CoverArtSize = 0; // small
    else if (height<=160) CoverArtSize = 1; // medium
    else CoverArtSize = 2; // large
}

/** Returns the number of matched records returned from the last Query() call.
 *
 *  @return    Number of matched records
 */
int CDbAmazon::NumberOfMatches() const
{
    return Releases.size();
}

/** Return the ID of the release
 *
 *  @param[in] Disc record ID (0-based index). If omitted, the first record (0)
 *             is returned.
 *  @return ID string
 */
std::string CDbAmazon::ReleaseId(const int recnum) const
{
    // check for the valid disc request
    if (Releases.empty() || recnum<0 || recnum>=(int)Releases.size())
        throw(std::runtime_error("Invalid CD record ID."));

    return Releases[recnum].ReleaseId();
}

/** Check if the query returned a front cover
 *
 *  @param[in]  record index (default=0)
 *  @return     true if front cover is found.
 */
bool CDbAmazon::Front(const int recnum) const
{
    // check for the valid disc request
    if (Releases.empty() || recnum<0 || recnum>=(int)Releases.size())
        throw(std::runtime_error("Invalid CD record ID."));

    return Releases[recnum].HasImage();
}

/** Get the URL of the front cover image
 *
 *  @param[in]  Record index (default=0)
 *  @return     URL string
 */
std::string CDbAmazon::FrontURL(const int recnum) const
{
    // check for the valid disc request
    if (Releases.empty() || recnum<0 || recnum>=(int)Releases.size())
        throw(std::runtime_error("Invalid CD record ID."));

    // get the URL of the image file
    return Releases[recnum].ImageURL(CoverArtSize);
}

/** Retrieve the front cover data.
 *
 *  @param[out] image data buffer.
 *  @param[in]  record index (default=0)
 */
UByteVector CDbAmazon::FrontData(const int recnum)
{
    // get the URL of the image file
    return DataToMemory(FrontURL(recnum));
}
