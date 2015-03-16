#include "enums.h"

#include <stdexcept>

using std::runtime_error;

/**
 * @brief Convert OutputFileFormat value to string
 * @param[in] OutputFileFormat value
 * @return a string with the representation of fmt
 * @throw std::runtime_error if associated string is not given for fmt
 */
std::string to_string(const OutputFileFormat fmt)
{
    switch (fmt)
    {
    case OutputFileFormat::WAVPACK: return "wavpack";
    default: throw(runtime_error("Unsupported format. Update std::string to_string(const OutputFileFormat fmt)"));
    }

    return "";
}

/**
 * @brief Convert RelaeseDatabase value to string
 * @param[in] RelaeseDatabase value
 * @return a string with the representation of db
 * @throw std::runtime_error if associated string is not given for db
 */
std::string to_string(const ReleaseDatabase db)
{
    switch (db)
    {
    case ReleaseDatabase::DISCOGS: return "discogs";
    case ReleaseDatabase::MUSICBRAINZ: return "musicbrainz";
    case ReleaseDatabase::FREEDB: return "freedb";
    case ReleaseDatabase::LASTFM: return "lastfm";
    default: throw(runtime_error("Unsupported database. Update std::string to_string(const ReleaseDatabase db)"));
    }

    return "";
}

/**
 * @brief Convert ImageDatabase value to string
 * @param[in] ImageDatabase value
 * @return a string with the representation of db
 * @throw std::runtime_error if associated string is not given for db
 */
std::string to_string(const ImageDatabase db)
{
    switch (db)
    {
    case ImageDatabase::MUSICBRAINZ: return "musicbrainz";
    case ImageDatabase::LASTFM: return "lastfm";
    default: throw(runtime_error("Unsupported database. Update std::string to_string(const ImageDatabase db)"));
    }

    return "";
}

/**
 * @brief Convert string to OutputFileFormat
 * @param[in] String object with the representation of OutputFileFormat value
 * @param[in] Pointer to an object of type size_t, whose value is set by the function to position
 *            of the next character in str after the OutputFileFormat value. This parameter can
 *            also be a null pointer, in which case it is not used.
 * @return On success, the function returns the converted value of OutputFileFormat type
 * @throw std::invalid_argument if no conversion could be performed
 */
OutputFileFormat sttooff(const std::string& str)
{
    if (str.compare(0,7,"wavpack")==0)
        return OutputFileFormat::WAVPACK;

    throw(runtime_error("Unsupported output file type name."));
}


/**
 * @brief Convert string to ReleaseDatabase
 * @param[in] String object with the representation of ReleaseDatabase value
 * @param[in] Pointer to an object of type size_t, whose value is set by the function to position
 *            of the next character in str after the ReleaseDatabase value. This parameter can
 *            also be a null pointer, in which case it is not used.
 * @return On success, the function returns the converted value of ReleaseDatabase type
 * @throw std::invalid_argument if no conversion could be performed
 */
ReleaseDatabase sttordb(const std::string& str)
{
    if (str.compare(0,7,"discogs")==0)
        return ReleaseDatabase::DISCOGS;
    else if (str.compare(0,11,"musicbrainz")==0)
        return ReleaseDatabase::MUSICBRAINZ;
    else if (str.compare(0,6,"freedb")==0)
        return ReleaseDatabase::FREEDB;
    else if (str.compare(0,6,"lastfm")==0)
        return ReleaseDatabase::LASTFM;

    throw(runtime_error("Unsupported database name."));
}

/**
 * @brief Convert string to ImageDatabase
 * @param[in] String object with the representation of ImageDatabase value
 * @param[in] Pointer to an object of type size_t, whose value is set by the function to position
 *            of the next character in str after the ImageDatabase value. This parameter can also
 *            be a null pointer, in which case it is not used.
 * @return On success, the function returns the converted value of ImageDatabase type
 * @throw std::invalid_argument if no conversion could be performed
 */
ImageDatabase sttoidb(const std::string& str, size_t* idx)
{
    if (str.compare(0,11,"musicbrainz")==0)
        return ImageDatabase::MUSICBRAINZ;
    else if (str.compare(0,6,"lastfm")==0)
        return ImageDatabase::LASTFM;

    throw(runtime_error("Unsupported database name."));
}
