#pragma once

#include <string>
#include <vector>

#define NumberOfOutputFileFormats 1
#define NumberOfDatabases 4

/**
 * @brief The OutputFileFormat enum
 */
enum class OutputFileFormat {WAVPACK};

/**
 * @brief The DatabaseType enum
 */
enum class DatabaseType {DISCOGS,MUSICBRAINZ,FREEDB,LASTFM,AMAZON};

/**
 * @brief The AlbumRemFieldType enum
 */
enum class AlbumRemFieldType
{
    DBINFO,     /// source database & ID
    GENRE,      /// music genre
    DATE,       /// release date
    COUNTRY,    /// release country
    UPC,        /// UPC barcode
    LABEL,      /// label
    CATNO,      /// catalog number
    DISC,       /// disc#
    DISCS,      /// number of discs in a set
};

typedef std::vector<AlbumRemFieldType> AlbumRemFieldVector;

/**
 * @brief Convert OutputFileFormat value to string
 * @param[in] OutputFileFormat value
 * @return a string with the representation of fmt
 * @throw std::runtime_error if associated string is not given for fmt
 */
std::string to_string(const OutputFileFormat fmt);

/**
 * @brief Convert RelaeseDatabase value to string
 * @param[in] RelaeseDatabase value
 * @return a string with the representation of db
 * @throw std::runtime_error if associated string is not given for db
 */
std::string to_string(const DatabaseType db);

/**
 * @brief Convert string to OutputFileFormat
 * @param[in] String object with the representation of OutputFileFormat value
 * @return On success, the function returns the converted value of OutputFileFormat type
 * @throw std::invalid_argument if no conversion could be performed
 */
OutputFileFormat sttooff(const std::string& str);

/**
 * @brief Convert string to DatabaseType
 * @param[in] String object with the representation of DatabaseType value
 * @return On success, the function returns the converted value of DatabaseType type
 * @throw std::invalid_argument if no conversion could be performed
 */
DatabaseType sttodb(const std::string& str);
