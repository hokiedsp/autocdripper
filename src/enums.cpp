#include "enums.h"

#include <stdexcept>

using std::runtime_error;


/**
 * @brief Convert OutputFileFormat value to file extension (incl. preceeding '.')
 * @param[in] OutputFileFormat value
 * @return a string with the representation of fmt
 * @throw std::runtime_error if associated string is not given for fmt
 */
std::string to_ext(const OutputFileFormat fmt)
{
    switch (fmt)
    {
    case OutputFileFormat::WAVPACK: return ".wv";
    case OutputFileFormat::CUE: return ".cue";
    default: throw(runtime_error("Unsupported format. Update std::string to_string(const OutputFileFormat fmt)"));
    }

    return "";
}

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
    case OutputFileFormat::CUE: return "cue";
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
std::string to_string(const DatabaseType db)
{
    switch (db)
    {
    case DatabaseType::DISCOGS: return "discogs";
    case DatabaseType::MUSICBRAINZ: return "musicbrainz";
    case DatabaseType::FREEDB: return "freedb";
    case DatabaseType::LASTFM: return "lastfm";
    case DatabaseType::AMAZON: return "amazon";
    default: throw(runtime_error("Unsupported database. Update std::string to_string(const DatabaseType db)"));
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
    else if (str.compare(0,3,"cue")==0)
        return OutputFileFormat::CUE;

    throw(runtime_error("Unsupported output file type name."));
}


/**
 * @brief Convert string to DatabaseType
 * @param[in] String object with the representation of DatabaseType value
 * @param[in] Pointer to an object of type size_t, whose value is set by the function to position
 *            of the next character in str after the DatabaseType value. This parameter can
 *            also be a null pointer, in which case it is not used.
 * @return On success, the function returns the converted value of DatabaseType type
 * @throw std::invalid_argument if no conversion could be performed
 */
DatabaseType sttodb(const std::string& str)
{
    if (str.compare(0,7,"discogs")==0)
        return DatabaseType::DISCOGS;
    else if (str.compare(0,11,"musicbrainz")==0)
        return DatabaseType::MUSICBRAINZ;
    else if (str.compare(0,6,"freedb")==0)
        return DatabaseType::FREEDB;
    else if (str.compare(0,6,"lastfm")==0)
        return DatabaseType::LASTFM;
    else if (str.compare(0,6,"amazon")==0)
        return DatabaseType::AMAZON;

    throw(runtime_error("Unsupported database name."));
}
