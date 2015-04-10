#include "CDbLastFmElem.h"

#include <stdexcept>
#include <cstdlib>
#include <climits>
//#include <ctime>
//#include <sstream>
//#include <iomanip>
//#include <numeric> // for std::accumulate

#include <iostream>

#include "utils.h"

CDbLastFmElem::CDbLastFmElem(const std::string &data)
    : CUtilJson(data)
{
}


/**
 * @brief Exchanges the content with another CDbLastFmElem object
 * @param Another CDbLastFmElem object
 */
void CDbLastFmElem::Swap(CDbLastFmElem &other)
{
    json_t *tempdata = data;
    data = other.data;
    other.data = tempdata;
}

/** Return a unique release ID string
 *
 *  @return id string if Query was successful.
 */
std::string CDbLastFmElem::ReleaseId() const
{
    std::string rval;
    FindString("id",rval);
    return rval;
}

/**
 * @brief Returns true if release data has a link to coverart images
 * @return true if there is a link
 */
bool CDbLastFmElem::HasImage() const
{
    bool rval = false;

    // must have image array object
    json_t *imarray, *imobject;
    size_t index;
    std::string url;
    if (FindArray("image",imarray))
    {
        // go through each image object (assume image array contains its immages in increasing size)
        json_array_foreach(imarray, index, imobject)
        {
            // if any image exists, return valid
            if (FindString(imobject,"#text",url))
            {
                rval = true;
                break;
            }
        }
    }
    return rval;
}

std::string CDbLastFmElem::ImageURL(int size) const
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
    std::string url, sizestr;
    size_t index;

    // must have image array object
    if (FindArray("image",imarray))
    {
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
            if (!FindString(imobject,"#text",url)) continue;

            // check the image type
            if (CompareString(imobject,"size",sizestr)==0) break;
        }
    }

    return url;
}
