#include "CDbMusicBrainzElemCAA.h"

#include <stdexcept>
#include <cstdlib>
#include <climits>
#include <cctype>

#include <iostream>

CDbMusicBrainzElemCAA::CDbMusicBrainzElemCAA(const std::string &rawdata)
    : CUtilJson(rawdata)
{
//    http://ia902607.us.archive.org/32/items/mbid-76df3287-6cda-33eb-8e9a-044b5e15ffdd/index.json
}

// -----------------------------------------------------------------------------------------

/** Return a unique release ID string
 *
 *  @return id string if Query was successful.
 */
std::string CDbMusicBrainzElemCAA::ReleaseId() const
{
    std::string rval;
    json_int_t rvalint;

    if (FindInt(data,"id",rvalint))
        rval = std::to_string(rvalint);

    return rval;
}

/** Get the URL of the front cover image
 *
 *  @return     URL string
 */
std::string CDbMusicBrainzElemCAA::FrontURL() const
{
    std::string rval;
    return rval;
}

/** Get the URL of the back cover image
 *
 *  @return     URL string
 */
std::string CDbMusicBrainzElemCAA::BackURL() const
{
    std::string rval;
    return rval;
}
