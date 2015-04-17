#include "CDbAmazonElem.h"

#include <stdexcept>
#include <cstdlib>
#include <climits>

#include <iostream>

#include "utils.h"

CDbAmazonElem::CDbAmazonElem(const std::string &newasin, const std::string &data)
    : CUtilXmlTree(data), asin(newasin)
{
    // change the root to release element
    if (root)
    {
        if (!FindElement("Document",root) && root)
            throw(std::runtime_error("MusicBrainz release lookup resulted in an invalid result."));
    }
}

/**
 * @brief Load New Data. Clears existing data first. If empty, no action.
 * @param valid XML string following musicbrainz_mmd-2.0.rng
 * @throw runtime_error if invalid XML string is passed in.
 */
void CDbAmazonElem::LoadData(const std::string &newasin, const std::string &rawdata)
{
    // call the base class function first
    CUtilXmlTree::LoadData(rawdata);

    // change the root to release element
    if (root && !FindElement("Document",root) && root)
        throw(std::runtime_error("MusicBrainz release lookup resulted in an invalid result."));

    // store the ASIN
    asin = newasin;
}

/** Return a unique release ID string
 *
 *  @return id string if Query was successful.
 */
std::string CDbAmazonElem::ReleaseId() const
{
    return asin;
}

/**
 * @brief Returns true if release data has a link to coverart images
 * @return true if there is a link
 */
bool CDbAmazonElem::HasImage() const
{
    const xmlNode *links, *ImgUrl;
    return root && FindElement("Links",links) && FindElement(links,"ImgUrl",ImgUrl);
}

/**
 * @brief ImageURL
 * @param size
 * @return
 */
std::string CDbAmazonElem::ImageURL(int size) const
{
    std::string rval;

    // start the traversal of tracklist
    const xmlNode *links, *images;
    if (root && FindElement("Links",links) && FindElement(links,"Images",images))
    {
        switch (size)
        {
        case 2:
            if (FindString(images,"LargeImage",rval)) break;
        case 1:
            if (FindString(images,"MediumImage",rval)) break;
        default:
            FindString(images,"SmallImage",rval);
        }
    }

    return rval;
}
