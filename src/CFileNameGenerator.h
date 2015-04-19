#pragma once

#include <string>

class SCueSheet;

/**
 * @brief A factor to generate a file name from populated cuesheet
 *
 * CFileNameGenerator generates a file name based on a populated cuesheet, according to
 * its file naming "scheme" public member variable. The file extension is automatically
 * appended (i.e., not part of the scheme) based on public member variable "fmt".
 *
 * Its naming scheme follows the foobar2000/CUETools convension:
 *
 *    * http://www.cuetools.net/wiki/Cuetools_templates
 *    * http://wiki.hydrogenaudio.org/index.php?title=Foobar2000:Titleformat_Reference
 *
 * Scheme Variables:
 *
 *    - %artist%
 *      Name of the artist of the album. Checks following metadata fields, in this order:
 *      "performer", "songwriter".
 *      Last name of the artist of the album. That is, the last contiguous non-space word
 *      returned by %artist%
 *    - %artist first%
 *      Use only the first artist name
 *    - %performer%
 *      Name of the performing artist of the album
 *    - %performer last%
 *      Last name of the performing artist of the album
 *    - %songwriter%
 *    - %songwriter last%
 *    - %album%
 *      Name of the album.
 *    - %discnumber%, %disc%
 *      Index of disc. Available only when "discnumber"/"disc" field is present in track’s metadata.
 *    - %totaldiscs%
 *      Index of total discs within the release.
 *    - Any REM field name (the first word after REM) can be used as a variable: "upc" "label"
 *
 * Scheme Flow Control
 *    - Text encolosed in brackets [...] is removed if any of the variables inside it are undefined.
 *
 * Scheme Syntax Bypass
 *    - Text enclosed in single quotation marks, e.g. '(%)' is inserted bypassing syntax processing;
 *      allows special characters such as %,$,[,] to be inserted.
 *    - In order to insert a quotation mark character, use '' (two single quotation marks).
 *
 * Scheme Function
 *

 */
class CFileNameGenerator
{
public:
    std::string scheme; /// path template
    OutputFileFormat fmt;    /// file extension

    /**
     * @brief Constructor
     * @param[in] file naming scheme string
     * @param[in] file format
     */
    CFileNameGenerator(const std::string &scheme, const OutputFileFormat &fmt);

    /**
     * @brief generate a file name from a cuesheet object
     * @param[in] populated cuesheet
     * @return generated file name string
     */
    std::string operator()(const SCueSheet &cuesheet);
};
