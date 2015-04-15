#include "CDbMusicBrainzElemCAA.h"

#include <stdexcept>
#include <cstdlib>
#include <climits>
#include <cctype>

#include <iostream>

CDbMusicBrainzElemCAA::CDbMusicBrainzElemCAA(const std::string &rawdata)
    : CUtilJson(rawdata)
{}

// -----------------------------------------------------------------------------------------

/** Return a unique release ID string
 *
 *  @return id string if Query was successful.
 */
std::string CDbMusicBrainzElemCAA::ReleaseId() const
{
    std::string rval;
    FindString(data,"release",rval);
    return rval;
}

/** Get the URL of the front cover image
 *
 *  @return     URL string
 */
std::string CDbMusicBrainzElemCAA::FrontURL(const int CoverArtSize) const
{
    return ImageURL_(CoverArtSize,"Front");
}

/** Get the URL of the back cover image
 *
 *  @return     URL string
 */
std::string CDbMusicBrainzElemCAA::BackURL(const int CoverArtSize) const
{
    return ImageURL_(CoverArtSize,"Back");
}

/**
 * @brief Get Image URL of specified type
 * @param[in]  image size 0-full, 1-large thumbnail (500px), 2-small thumbnail (250px)
 * @param type ["Front","Medium","Back"
 * @return
 */
std::string CDbMusicBrainzElemCAA::ImageURL_(const int CoverArtSize, const std::string &typeneed) const
{
    std::string rval;

    // start the traversal of tracklist
    json_t *images, *image;
    bool gotonext = true;

    // get the images array
    if (FindArray("images", images))
    {

        // for each image of the images
        for(size_t index = 0;
            gotonext && index < json_array_size(images) && (image= json_array_get(images, index));
            index++)
        {
            bool istype;
            if (FindBool(image,typeneed,istype) && istype)
            {
                gotonext = false; // found the match

                if (CoverArtSize==0) // full-size image
                {
                    FindString(image,"image",rval);
                }
                else
                {
                    json_t *thumbnails;
                    if (FindObject(image,"thumbnails",thumbnails))
                    {
                        if (CoverArtSize==1) // large
                            FindString(thumbnails,"large",rval);
                        else // small
                            FindString(thumbnails,"small",rval);
                    }
                }
            }
        }
    }
    return rval;
}
