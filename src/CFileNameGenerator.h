#pragma once

#include <string>

#include "enums.h"
#include "SCueSheet.h"

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
 *    * http://wiki.hydrogenaud.io/index.php?title=Foobar2000:Titleformat_Reference
 *
 * Scheme Variables:
 *
 * - %artist%
 *      Name of the artist of the album. Checks following metadata fields, in this order:
 *      "performer", "songwriter".
 * - %performer%
 *      Name of the performing artist of the album
 * - %songwriter%
 *      Name of the composer of the album
 * - %artist first%, %performer first%, %songwriter first%
 *      Name of the first artist of the artist
 * - %artist lastname%, %performer lastname%, %songwriter lastname%
 *      Use only the last name of the first artist name (use with caution as it does not distinguish
 *      person and group names
 * - %album%
 *      Name of the album.
 * - %discnumber%, %disc%
 *      Index of disc. Available only when "discnumber"/"disc" field is present in track’s metadata.
 * - %totaldiscs%, %discs%
 *      Index of total discs within the release.
 * - Any REM field name (the first word after REM) can be used as a variable: "upc" "label"
 *
 * Scheme Flow Control
 *    - Text encolosed in brackets [...] is removed if any of the variables inside it are undefined.
 *
 * Scheme Syntax Bypass
 *    - Text enclosed in single quotation marks, e.g. '(%)' is inserted bypassing syntax processing;
 *      allows special characters such as %,$,[,] to be inserted.
 *    - In order to insert a quotation mark character, use '' (two single quotation marks).
 *
 * Scheme Functions
 *    -
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
    std::string operator()(const SCueSheet &cuesheet) const;

private:
    typedef struct SParserOutput
    {
        std::string str;
        bool tf;
        size_t end;

        SParserOutput() : tf(false), end(str.npos) { str.reserve(256); }
    } ParserOutput;

    /**
     * @brief (recursive) working function for operator()
     * @param[in] populated cuesheet
     * @param[in] starting position of scheme string
     * @param[in] a list of terminating characters
     * @return generated file name string
     */
    ParserOutput parser(const SCueSheet &cuesheet, size_t pos0=0,
                        const std::string &termch="") const;

    /**
     * @brief Find REM field value
     * @param[in] populated cuesheet
     * @param[in] REM field name (first word following REM)
     * @return REM field value
     */
    std::string FindRem_(const SCueSheet &cuesheet, const std::string &rem_type) const;

    /**
     * @brief Generate name string according to the specified format option
     * @param[in] list of artists to be concatenated
     * @param[in] number of artists to include (<=0 to include all)
     * @param[in] 0-full name, 1 last name only, >=2-first initial + last name
     * @return formatted name string
     */
    std::string FormName_(const SCueArtists &artists, const int numlist, const int option) const;

    /**
     * @brief Extract last name (i.e., the word(s) appear at the end of string)
     * @param[in] fullname string
     * @param[out] (Optional) iterator at the first non-word character preceeding
     *             the last name (use this as the end point of the initial search)
     * @return string containing just last name
     */
    static std::string LastName_(const std::string &fullname,
                                 std::string::const_iterator *start=nullptr);
//                                 std::string::const_iterator *&start);
//    static std::string LastName_(std::string::const_iterator start,
//                                 std::string::const_iterator end);

    /**
     * @brief Extract initials + last name
     * @param[in] fullname string
     * @return string containing initials (no space between) + lastname
     */
    static std::string InitialsPlusLastName_(const std::string &fullname);

    /**
     * @brief Analyze non-word characters for brackets and quotations
     * @param[in] start of the non-word section
     * @param[in] end of the non-word section
     * @param[inout] first/end character is inside of this # of bracket pairs
     */
    static void CheckBrackets_(const std::string::const_iterator begin,
                               const std::string::const_iterator end,
                               int &inbracket);
};
