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
std::string to_string(const ReleaseDatabaseType db)
{
    switch (db)
    {
    case ReleaseDatabaseType::DISCOGS: return "discogs";
    case ReleaseDatabaseType::MUSICBRAINZ: return "musicbrainz";
    case ReleaseDatabaseType::FREEDB: return "freedb";
    case ReleaseDatabaseType::LASTFM: return "lastfm";
    default: throw(runtime_error("Unsupported database. Update std::string to_string(const ReleaseDatabaseType db)"));
    }

    return "";
}

/**
 * @brief Convert ImageDatabaseType value to string
 * @param[in] ImageDatabaseType value
 * @return a string with the representation of db
 * @throw std::runtime_error if associated string is not given for db
 */
std::string to_string(const ImageDatabaseType db)
{
    switch (db)
    {
    case ImageDatabaseType::MUSICBRAINZ: return "musicbrainz";
    case ImageDatabaseType::LASTFM: return "lastfm";
    default: throw(runtime_error("Unsupported database. Update std::string to_string(const ImageDatabaseType db)"));
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
 * @brief Convert string to ReleaseDatabaseType
 * @param[in] String object with the representation of ReleaseDatabaseType value
 * @param[in] Pointer to an object of type size_t, whose value is set by the function to position
 *            of the next character in str after the ReleaseDatabaseType value. This parameter can
 *            also be a null pointer, in which case it is not used.
 * @return On success, the function returns the converted value of ReleaseDatabaseType type
 * @throw std::invalid_argument if no conversion could be performed
 */
ReleaseDatabaseType sttordb(const std::string& str)
{
    if (str.compare(0,7,"discogs")==0)
        return ReleaseDatabaseType::DISCOGS;
    else if (str.compare(0,11,"musicbrainz")==0)
        return ReleaseDatabaseType::MUSICBRAINZ;
    else if (str.compare(0,6,"freedb")==0)
        return ReleaseDatabaseType::FREEDB;
    else if (str.compare(0,6,"lastfm")==0)
        return ReleaseDatabaseType::LASTFM;

    throw(runtime_error("Unsupported database name."));
}

/**
 * @brief Convert string to ImageDatabaseType
 * @param[in] String object with the representation of ImageDatabaseType value
 * @param[in] Pointer to an object of type size_t, whose value is set by the function to position
 *            of the next character in str after the ImageDatabaseType value. This parameter can also
 *            be a null pointer, in which case it is not used.
 * @return On success, the function returns the converted value of ImageDatabaseType type
 * @throw std::invalid_argument if no conversion could be performed
 */
ImageDatabaseType sttoidb(const std::string& str, size_t* idx)
{
    if (str.compare(0,11,"musicbrainz")==0)
        return ImageDatabaseType::MUSICBRAINZ;
    else if (str.compare(0,6,"lastfm")==0)
        return ImageDatabaseType::LASTFM;

    throw(runtime_error("Unsupported database name."));
}
