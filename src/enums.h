#pragma once

#include <string>

#define NumberOfOutputFileFormats 1
#define NumberOfReleaseDatabases 4
#define NumberOfImageDatabases 2

enum class OutputFileFormat {WAVPACK};
enum class ReleaseDatabaseType {DISCOGS,MUSICBRAINZ,FREEDB,LASTFM};
enum class ImageDatabaseType {MUSICBRAINZ,LASTFM};

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
std::string to_string(const ReleaseDatabaseType db);

/**
 * @brief Convert ImageDatabaseType value to string
 * @param[in] ImageDatabaseType value
 * @return a string with the representation of db
 * @throw std::runtime_error if associated string is not given for db
 */
std::string to_string(const ImageDatabaseType db);

/**
 * @brief Convert string to OutputFileFormat
 * @param[in] String object with the representation of OutputFileFormat value
 * @return On success, the function returns the converted value of OutputFileFormat type
 * @throw std::invalid_argument if no conversion could be performed
 */
OutputFileFormat sttooff(const std::string& str);

/**
 * @brief Convert string to ReleaseDatabaseType
 * @param[in] String object with the representation of ReleaseDatabaseType value
 * @return On success, the function returns the converted value of ReleaseDatabaseType type
 * @throw std::invalid_argument if no conversion could be performed
 */
ReleaseDatabaseType sttordb(const std::string& str);

/**
 * @brief Convert string to ImageDatabase
 * @param[in] String object with the representation of ImageDatabaseType value
 * @return On success, the function returns the converted value of ImageDatabaseType type
 * @throw std::invalid_argument if no conversion could be performed
 */
ImageDatabaseType sttoidb(const std::string& str);
