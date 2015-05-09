#pragma once

#include <string>
#include <vector>

/**
 *
 */
namespace utf8fcns
{

/**
 * @brief Returns abbreviation of x
 * @param string to be abbreviated
 * @param (Optional) Only abbreviate if x is longer than len (default=0: abbreviate all)
 * @return abbreviated string
 */
std::string abbr(std::string x, const size_t len=0);

/**
 * @brief Converts first letter of first word to a capital letter and all other letters to lower case
 * @param input string
 * @return capitalized string
 */
std::string cap(std::string x);

/**
 * @brief Converts first letter of first word to a capital letter. All other letters are kept the same
 * @param Input string
 * @return Capitalized string
 */
std::string cap2(std::string x);

/**
 * @brief Converts first letter of every word to a capital letter and all other letters to lower case
 * @param input string
 * @return capitalized string
 */
std::string caps(std::string x);

/**
 * @brief Converts first letter of every word to a capital letter. All other letters are kept the same
 * @param input string
 * @return capitalized string
 */
std::string caps2(std::string x);

/**
 * @brief Returns first len characters from the left of the string a.
 * @param Input string
 * @param Number of characters to return. A negative number produces the entire string.
 * @return Truncated string
 */
std::string cut(const std::string &a, const size_t len);

/**
 * @brief truncate words beyond (len)-th character
 * @param input string
 * @param len (characters)
 * @return truncated string
 */
std::string cutwords(const std::string &a, const size_t len);

/**
 * @brief Inserts b into a after n characters.
 * @param Base string
 * @param String to be inserted
 * @param Insertion point
 * @return
 */
std::string insert(const std::string &a, const std::string &b, const size_t n);

/**
 * @brief Returns length of string a in characters (equiv to foobar2000 len2)
 * @param Input string
 * @return Number of characters
 */
size_t len(const std::string &a);

/**
 * @brief Compare which string is longer
 * @param First string
 * @param Second string
 * @return True if the first string is longer than the second
 */
bool longer(const std::string &a, const std::string &b);

/**
 * @brief Get the longest string
 * @param Vector of input strings
 * @return Longest string
 */
std::string longest(const std::vector<std::string> &strs);

/**
 * @brief Converts a to lower case
 * @param Input string
 * @return Converted string
 */
std::string lower(const std::string &a);

/**
 * @brief replace
 * @param a
 * @param b
 * @param c
 * @return
 */
std::string replace(std::string a, std::string b, const std::string c);

/**
 * @brief Get the shortest string
 * @param Vector of input strings
 * @return Longest string
 */
std::string shortest(const std::vector<std::string> &strs);

/**
 * @brief Find first occurence of a character in a string
 * @param Input string
 * @param Input character
 * @return Byte location of the character (0-based)
 */
size_t strchr(const std::string &a, const char c);

/**
 * @brief Find last occurence of a character in a string
 * @param Input string
 * @param Input character
 * @return Byte location of the character (0-based)
 */
size_t strrchr(const std::string &a, const char c);

/**
 * @brief Find first occurence of a substring in a string
 * @param Input string
 * @param Input substring
 * @return Byte location of the starting character of the substring (0-based)
 */
size_t strstr(const std::string &s1, const std::string &s2);

/**
 * @brief Find last occurence of a substring in a string
 * @param Input string
 * @param Input substring
 * @return Byte location of the starting character of the substring (0-based)
 */
size_t strrstr(const std::string &s1, const std::string &s2);

/**
 * @brief Case-sensitive comparison of two strings
 * @param Input string
 * @param Input string
 * @return True if they are the same
 */
bool strcmp(const std::string &a, const std::string &b);

/**
 * @brief Case-insensitive comparison of two string
 * @param Input string
 * @param Input string
 * @return True if they are the same
 */
bool stricmp(const std::string &a, const std::string &b);

/**
 * @brief Get a substring
 * @param Input string
 * @param Substring starting byte index (0-based)
 * @param Substring length (number of characters)
 * @return Substring
 */
std::string substr(const std::string &a, const size_t m, const size_t len);

/**
 * @brief Removes prefixes from string
 * @param Input string
 * @param prefixes (if not given, defaults to "a" and "the")
 * @return String without the matched prefix
 */
std::string stripprefix(std::string x,
                        const std::vector<std::string> &prefixes = {"a","the"});

/**
 * @brief Move prefixes of string to the end
 * @param Input string
 * @param prefixes (if not given, defaults to "a" and "the")
 * @return String with the matched prefix appearing at the end (preceeded by ',')
 */
std::string swapprefix(std::string x,
                       const std::vector<std::string> &prefixes = {"a","the"});

/**
 * @brief Remove leading and trailing spaces
 * @param Input string
 * @return White-space free strings
 */
std::string trim(const std::string &s);

/**
 * @brief Convert all characters to upper case
 * @param Input string
 * @return Converted string
 */
std::string upper(const std::string &s);

}
