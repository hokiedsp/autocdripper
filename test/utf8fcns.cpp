#include "utf8fcns.h"

#include <boost/regex.hpp>
#include <boost/regex/icu.hpp>
#include <unicode/unistr.h>
#include <unicode/regex.h>

#include <sstream>

#include <iostream>

// "private" utf8fcns namespace static variables
namespace utf8fcns
{

static const std::string word_pattern("\\w+");
static const boost::u32regex word_exp = boost::make_u32regex(word_pattern);

static const std::string wordinit_pattern("(\\w)(\\w*)");
static const boost::u32regex wordinit_exp = boost::make_u32regex(wordinit_pattern);

static const std::string space_pattern("\\s");
static const boost::u32regex space_exp = boost::make_u32regex(space_pattern);

static const std::string perl_pattern("[\\.\\[\\{\\}\\(\\)\\\\\\*\\+\\?\\|\\^\\$]");
static const boost::u32regex perl_exp = boost::make_u32regex(perl_pattern);

std::string form_prefix_pattern(const std::vector<std::string> &prefixes)
{
    std::string prefix;
    std::string prefix_pattern("^\\s*(");

    std::vector<std::string>::const_iterator it=prefixes.begin();

    prefix = *it;
    boost::u32regex_replace(prefix,utf8fcns::perl_exp,"\\$1");
    prefix_pattern += prefix;

    for (++it; it!=prefixes.end(); ++it)
    {
        prefix = *it;
        boost::u32regex_replace(prefix,utf8fcns::perl_exp,"\\$1");
        prefix_pattern += '|' + prefix;
    }
    prefix_pattern += ")\\s+";

    return prefix_pattern;
}
}

/**
 * @brief Returns abbreviation of x
 * @param string to be abbreviated
 * @param (Optional) Only abbreviate if x is longer than len (default=0: abbreviate all)
 * @return abbreviated string
 */
std::string utf8fcns::abbr(const std::string &x, const size_t len)
{
    std::string rval;

    if (len == 0 || utf8fcns::len(x) > len)
    {
        // keep only the first letter of words
        rval = boost::u32regex_replace(x,utf8fcns::wordinit_exp,"$1");
        // eliminate all spaces
        rval = boost::u32regex_replace(rval,utf8fcns::space_exp,"");
    }
    else
    {
        rval = x;
    }

    return rval;
}

/**
 * @brief Converts first letter of first word to a capital letter and all other letters to lower case
 * @param input string
 * @return capitalized string
 */
std::string utf8fcns::cap(const std::string &x)
{
    // lowercase all characters
    std::string alllow = boost::u32regex_replace(x,utf8fcns::word_exp,"\\L$&\\E");

    // capitalize first character
    return boost::u32regex_replace(alllow,utf8fcns::wordinit_exp,"\\u$1\\L$2\\E",boost::format_first_only);
}

/**
 * @brief Converts first letter of first word to a capital letter. All other letters are kept the same
 * @param input string
 * @return capitalized string
 */
std::string utf8fcns::cap2(const std::string &x)
{
    std::string rval;
    boost::match_results<std::string::iterator> what;

    // capitalize first character
    return boost::u32regex_replace(x,utf8fcns::wordinit_exp,"\\u$1$2",boost::format_first_only);
}

/**
 * @brief Converts first letter of every word to a capital letter and all other letters to lower case
 * @param input string
 * @return capitalized string
 */
std::string utf8fcns::caps(const std::string &x)
{
    return u32regex_replace(x, utf8fcns::wordinit_exp, "\\u$1\\L$2\\E");
}

/**
 * @brief Converts first letter of every word to a capital letter. All other letters are kept the same
 * @param input string
 * @return capitalized string
 */
std::string utf8fcns::caps2(const std::string &x)
{
    return u32regex_replace(x, utf8fcns::word_exp, "\\u$&");
}

/**
 * @brief Returns first len characters from the left of the string a.
 * @param Input string
 * @param Number of characters to return. A negative number produces the entire string.
 * @return Truncated string
 */
std::string utf8fcns::cut(const std::string &a, const size_t len)
{
    std::string pattern = "^.{1," + std::to_string(len) + "}";
    boost::u32regex exp = boost::make_u32regex(pattern);
    boost::match_results<std::string::const_iterator> what;

    // format first word of x: first letter, upper-case, rest lower-case
    if (u32regex_search(a.begin(), a.end(), what, exp))
        return std::string(what[0].first,what[0].second);
    else
        return a;
}

/**
 * @brief truncate words beyond (len)-th character
 * @param input string
 * @param len (characters)
 * @return truncated string
 */
std::string utf8fcns::cutwords(const std::string &a, const size_t len)
{
    std::string pattern = "^(.{1," + std::to_string(len) + "})\\W";
    boost::u32regex exp = boost::make_u32regex(pattern);
    boost::match_results<std::string::const_iterator> what;

    // format first word of x: first letter, upper-case, rest lower-case
    if (u32regex_search(a.begin(), a.end(), what, exp))
        return std::string(what[0].first,what[1].second);
    else // first word is longer than len, cut the word
        return utf8fcns::cut(a,len);
}

/**
 * @brief Inserts b into a after n characters.
 * @param Base string
 * @param String to be inserted
 * @param Insertion point
 * @return
 */
std::string utf8fcns::insert(const std::string &a, const std::string &b, const size_t n)
{
    // perform the function in UnicodeString
    icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(a);
    icu::UnicodeString ustrsrc = icu::UnicodeString::fromUTF8(b);
    ustr.insert(n,ustrsrc);

    // convert the result back in UTF-8
    std::string rval;
    ustr.toUTF8String(rval);
    return rval;
}

/**
 * @brief Returns length of string a in characters (equiv to foobar2000 len2)
 * @param Input string
 * @return Number of characters
 */
size_t utf8fcns::len(const std::string &a)
{
    icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(a);
    return ustr.countChar32();
}

/**
 * @brief Compare which string is longer
 * @param First string
 * @param Second string
 * @return True if the first string is longer than the second
 */
bool utf8fcns::longer(const std::string &a, const std::string &b)
{
    size_t Na = utf8fcns::len(a);
    return Na>utf8fcns::len(b);
}

/**
 * @brief Get the longest string
 * @param Vector of input strings
 * @return Longest string
 */
std::string utf8fcns::longest(const std::vector<std::string> &strs)
{
    std::string rval;
    std::vector<std::string>::const_iterator it, it0;
    size_t len = 0;
    if (strs.size())
    {
        for (strs.begin(); it!=strs.end(); ++it)
        {
            size_t l = utf8fcns::len(*it);
            if (l>=len)
            {
                len = l;
                it0 = it;
            }
        }

        rval = *it0;
    }
    return rval;
}

/**
 * @brief Converts a to lower case
 * @param Input string
 * @return Converted string
 */
std::string utf8fcns::lower(const std::string &a)
{
    // perform the function in UnicodeString
    icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(a);
    ustr.toLower();

    // convert the result back in UTF-8
    std::string rval;
    ustr.toUTF8String(rval);
    return rval;
}

/**
 * @brief Replace all occurrence of string b in string a with string c
 * @param Input string
 * @param Substring to be replaced
 * @param Replacing substring
 * @return Replaced string
 */
std::string utf8fcns::replace(std::string a, std::string b, const std::string c)
{
    // make sure that b does not contain any regex special characters
    // if so, add preceding backslash to look for all specifal characters as themselves
    boost::u32regex_replace(b,utf8fcns::perl_exp,"\\$1");

    // run regex_replace
    boost::u32regex rep_exp = boost::make_u32regex(b);
    boost::u32regex_replace(a,rep_exp,c);

    return a;
}

/**
 * @brief Get the shortest string
 * @param Vector of input strings
 * @return Longest string
 */
std::string utf8fcns::shortest(const std::vector<std::string> &strs)
{
    std::string rval;
    std::vector<std::string>::const_iterator it, it0;
    size_t len = 0;

    if (strs.size())
    {
        for (strs.begin(); it!=strs.end(); ++it)
        {
            size_t l = utf8fcns::len(*it);
            if (l<=len)
            {
                len = l;
                it0 = it;
            }
        }

        rval = *it0;
    }
    return rval;
}

/**
 * @brief Find first occurence of a character in a string
 * @param Input string
 * @param Input character
 * @return Byte location of the character (0-based)
 */
size_t utf8fcns::strchr(const std::string &a, const char c)
{
    return a.find_first_of(c);
}

/**
 * @brief Find last occurence of a character in a string
 * @param Input string
 * @param Input character
 * @return Byte location of the character (0-based)
 */
size_t utf8fcns::strrchr(const std::string &a, const char c)
{
    return a.find_last_of(c);
}

/**
 * @brief Find first occurence of a substring in a string
 * @param Input string
 * @param Input substring
 * @return Byte location of the starting character of the substring (0-based)
 */
size_t utf8fcns::strstr(const std::string &s1, const std::string &s2)
{
    return s1.find(s2);
}

/**
 * @brief Find last occurence of a substring in a string
 * @param Input string
 * @param Input substring
 * @return Byte location of the starting character of the substring (0-based)
 */
size_t utf8fcns::strrstr(const std::string &s1, const std::string &s2)
{
    return s1.rfind(s2);
}

/**
 * @brief Case-sensitive comparison of two strings
 * @param Input string
 * @param Input string
 * @return True if they are the same
 */
bool utf8fcns::strcmp(const std::string &a, const std::string &b)
{
    return 0 == a.compare(b);
}

/**
 * @brief Case-insensitive comparison of two string
 * @param Input string
 * @param Input string
 * @return True if they are the same
 */
bool utf8fcns::stricmp(const std::string &a, const std::string &b)
{
    icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(a);
    return 0 == ustr.caseCompare(icu::UnicodeString::fromUTF8(b),
                                 U_FOLD_CASE_DEFAULT);
}

/**
 * @brief Get a substring
 * @param Input string
 * @param Substring starting byte index (0-based)
 * @param Substring length (number of characters)
 * @return Substring
 */
std::string utf8fcns::substr(const std::string &a, const size_t m, const size_t len)
{
    icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(a);
    icu::UnicodeString utgtstr;
    ustr.extract(m,len,utgtstr);

    // convert the result back in UTF-8
    std::string rval;
    utgtstr.toUTF8String(rval);
    return rval;
}

/**
 * @brief Removes prefixes from string
 * @param Input string
 * @param prefixes (if not given, defaults to "a" and "the")
 * @return String without the matched prefix
 */
std::string utf8fcns::stripprefix(std::string x, const std::vector<std::string> &prefixes)
{
    std::string prefix_pattern = utf8fcns::form_prefix_pattern(prefixes);
    boost::u32regex prefix_exp = boost::make_u32regex(prefix_pattern,boost::regex::perl|boost::regex::icase);
    u32regex_replace(x, prefix_exp, "", boost::format_first_only);

    return x;
}

/**
 * @brief Move prefixes of string to the end
 * @param Input string
 * @param prefixes (if not given, defaults to "a" and "the")
 * @return String with the matched prefix appearing at the end (preceeded by ',')
 */
std::string utf8fcns::swapprefix(std::string x, const std::vector<std::string> &prefixes)
{
    std::string prefix_pattern = utf8fcns::form_prefix_pattern(prefixes);
    boost::u32regex prefix_exp = boost::make_u32regex(prefix_pattern,
                                                      boost::regex::perl|boost::regex::icase);

    boost::match_results<std::string::iterator> what;

    // format first word of x: first letter, upper-case, rest lower-case
    if (u32regex_search(x.begin(), x.end(), what, prefix_exp))
        what.format("$', $1");

    return x;
}

/**
 * @brief Remove leading and trailing spaces
 * @param Input string
 * @return White-space free strings
 */
std::string utf8fcns::trim(const std::string &s)
{
    // perform the function in UnicodeString
    icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(s);
    ustr.trim();

    // convert the result back in UTF-8
    std::string rval;
    ustr.toUTF8String(rval);
    return rval;
}

/**
 * @brief Convert all characters to upper case
 * @param Input string
 * @return Converted string
 */
std::string utf8fcns::upper(const std::string &s)
{
    // perform the function in UnicodeString
    icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(s);
    ustr.toUpper();

    // convert the result back in UTF-8
    std::string rval;
    ustr.toUTF8String(rval);
    return rval;
}
