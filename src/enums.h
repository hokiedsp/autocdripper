#pragma once

#include <string>

#define NumberOfOutputFileFormats 1
#define NumberOfReleaseDatabases 4
#define NumberOfImageDatabases 2

enum class OutputFileFormat {WAVPACK};
enum class ReleaseDatabase {DISCOGS,MUSICBRAINZ,FREEDB,LASTFM};
enum class ImageDatabase {MUSICBRAINZ,LASTFM};

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
std::string to_string(const ReleaseDatabase db);

/**
 * @brief Convert ImageDatabase value to string
 * @param[in] ImageDatabase value
 * @return a string with the representation of db
 * @throw std::runtime_error if associated string is not given for db
 */
std::string to_string(const ImageDatabase db);

/**
 * @brief Convert string to OutputFileFormat
 * @param[in] String object with the representation of OutputFileFormat value
 * @param[in] Pointer to an object of type size_t, whose value is set by the function to position
 *            of the next character in str after the OutputFileFormat value. This parameter can
 *            also be a null pointer, in which case it is not used.
 * @return On success, the function returns the converted value of OutputFileFormat type
 * @throw std::invalid_argument if no conversion could be performed
 */
OutputFileFormat sttooff(const std::string& str, size_t* idx = 0);

/**
 * @brief Convert string to ReleaseDatabase
 * @param[in] String object with the representation of ReleaseDatabase value
 * @param[in] Pointer to an object of type size_t, whose value is set by the function to position
 *            of the next character in str after the ReleaseDatabase value. This parameter can
 *            also be a null pointer, in which case it is not used.
 * @return On success, the function returns the converted value of ReleaseDatabase type
 * @throw std::invalid_argument if no conversion could be performed
 */
ReleaseDatabase sttordb(const std::string& str, size_t* idx = 0);

/**
 * @brief Convert string to ImageDatabase
 * @param[in] String object with the representation of ImageDatabase value
 * @param[in] Pointer to an object of type size_t, whose value is set by the function to position
 *            of the next character in str after the ImageDatabase value. This parameter can also
 *            be a null pointer, in which case it is not used.
 * @return On success, the function returns the converted value of ImageDatabase type
 * @throw std::invalid_argument if no conversion could be performed
 */
ImageDatabase sttoidb(const std::string& str, size_t* idx = 0);
