#pragma once

#include <string>

#include "CUtilJson.h"

class CDbMusicBrainzElemCAA : protected CUtilJson
{
    friend class CDbDiscogs;
public:
    CDbMusicBrainzElemCAA(const std::string &data);
    virtual ~CDbMusicBrainzElemCAA() {}

    /** Return a unique release ID string
     *
     *  @return id string if Query was successful.
     */
    std::string ReleaseId() const;

    /** Get the URL of the front cover image
     *
     *  @param[in]  image size 0-full, 1-large thumbnail (500px), 2-small thumbnail (250px)
     *  @return     URL string
     */
    std::string FrontURL(const int CoverArtSize) const;

    /** Get the URL of the back cover image
     *
     *  @param[in]  image size 0-full, 1-large thumbnail (500px), 2-small thumbnail (250px)
     *  @return     URL string
     */
    std::string BackURL(const int CoverArtSize) const;

private:

};
