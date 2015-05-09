#include "utf8fcns.h"

#include <boost/regex.hpp>
#include <boost/regex/icu.hpp>
#include <unicode/unistr.h>

// "private" utf8fcns namespace static variables
namespace utf8fcns
{
static const std::string word_pattern("\\w+");
static const boost::u32regex wordinit_exp = boost::make_u32regex(word_pattern);

static const std::string wordinit_pattern("(\\w)(\\w*)");
static const boost::u32regex wordinit_exp = boost::make_u32regex(wordinit_pattern);

static const std::string space_pattern("\\s");
static const boost::u32regex word_exp = boost::make_u32regex(space_pattern);

static const std::string perl_pattern("[\.\[\{\}\(\)\\\*\+\?\|\^\$]");
static const boost::u32regex perl_exp = boost::make_u32regex(perl_pattern);

std::string form_prefix_pattern(const std::vector<std::string> &prefixes)
{
    std::string prefix;
    std::string prefix_pattern("^\\s*(");

    std::vector<std::string>::const_iterator it=prefixes.begin();

    prefix = *it;
    boost::u32regex_replace(prefix.begin(),prefix.end(),utf8fcns::perl_exp,"\\$1");
    prefix_pattern += prefix;

    for (++it; it!=prefixes.end(); ++it)
    {
        prefix = *it;
        boost::u32regex_replace(prefix.begin(),prefix.end(),utf8fcns::perl_exp,"\\$1");
        prefix_pattern += '|' + prefix;
    }
    prefix_pattern += ")\\s+";

    return prefix_pattern;
}

/**
 * @brief Returns abbreviation of x
 * @param string to be abbreviated
 * @param (Optional) Only abbreviate if x is longer than len (default=0: abbreviate all)
 * @return abbreviated string
 */
std::string utf8fcns::abbr(std::string x, const size_t len)
{
    if (len > 0 && len < utf8fcns::len(x))
    {
        // keep only the first letter of words
        boost::u32regex_replace(x.begin(),x.end(),utf8fcns::wordinit_exp,"$1");
        // eliminate all spaces
        boost::u32regex_replace(x.begin(),x.end(),utf8fcns::space_pattern,"");
    }
    return x;
}

/**
 * @brief Converts first letter of first word to a capital letter and all other letters to lower case
 * @param input string
 * @return capitalized string
 */
std::string utf8fcns::cap(std::string x)
{
    boost::match_results<std::string::const_iterator> what;

    // Run a single search to look for the first word. If success, format the first
    // letter of the word to be capitalized.
    if (u32regex_search(x.start, x.end, what, utf8fcns::wordinit_exp))
    {
        // format first word of x: first letter, upper-case, rest lower-case
        what.format("\u$1\L$2\E");

        // search again just in case (to account for multi-byte letters)
        if (u32regex_search(x.start, x.end, what, utf8fcns::word_exp))
            // force lower-case letters on all subsequent words
            boost::u32regex_replace(what[0].last,x.end(),utf8fcns::word_exp,"\L$&\E");
    }

    return x;
}

/**
 * @brief Converts first letter of first word to a capital letter. All other letters are kept the same
 * @param input string
 * @return capitalized string
 */
std::string utf8fcns::cap2(const std::string &x)
{
    boost::match_results<std::string::const_iterator> what;

    // Run a single search to look for the first word. If success, format the first
    // letter of the word to be capitalized.
    if (u32regex_search(x.start, x.end, what, utf8fcns::wordinit_exp))
        what.format("\u$1$2");

    return x;
}

/**
 * @brief Converts first letter of every word to a capital letter and all other letters to lower case
 * @param input string
 * @return capitalized string
 */
std::string utf8fcns::caps(std::string x)
{
    u32regex_replace(x.start, x.end, utf8fcns::wordinit_exp, "\u$1\L$2\E");
    return x;
}

/**
 * @brief Converts first letter of every word to a capital letter. All other letters are kept the same
 * @param input string
 * @return capitalized string
 */
std::string utf8fcns::caps2(std::string x)
{
    u32regex_replace(x.start, x.end, utf8fcns::word_exp, "\u$&");
    return x;
}

/**
 * @brief Returns first len characters from the left of the string a.
 * @param Input string
 * @param Number of characters to return. A negative number produces the entire string.
 * @return Truncated string
 */
std::string utf8fcns::cut(const std::string &a, const size_t len)
{
    std::string rval;
    if (len<0)
    {
        rval = a;
    }
    else if (len>0)
    {
        // perform the function in UnicodeString
        icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(a);
        ustr.truncate(len);

        // convert the result back in UTF-8
        ustr.toUTF8String(rval);
    }

    return rval;
}

/**
 * @brief truncate words beyond (len)-th character
 * @param input string
 * @param len (characters)
 * @return truncated string
 */
std::string utf8fcns::cutwords(const std::string &a, const size_t len)
{
    boost::u32regex_iterator<std::string::const_iterator> iend;
    boost::u32regex_iterator<std::string::const_iterator>
            i(boost::make_u32regex_iterator(str, utf8fcns::word_exp));
    std::string::const_iterator aend = a.begin();
    icu::UnicodeString ustr;
    size_t Nch = 0;

    while (i != iend && Nch<=len)
    {
        // convert
        ustr = icu::UnicodeString::fromUTF8(icu::StringPiece(aend,(*i)[0].last-aend));

        // get length
        Nch += ustr.countChar32();

        // if within the character limit, update the a iterator
        if (Nch<=len) a0ptr = (*i)[0].last;
        ++i;
    }

    return std::string(a.begin(),aend);
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
    std::vectro::const_iterator it, it0;
    size_t len = 0;
    for (strs.begin(); it!=strs.end(); ++it)
    {
        size_t l = utf8fcns::len(*it);
        if (l>=len)
        {
            len = l;
            it0 = it;
        }
    }

    return *it0;
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
    boost::u32regex_replace(b.begin(),b.end(),utf8fcns::perl_exp,"\\$1");

    // run regex_replace
    boost::u32regex_replace(a.begin(),a.end(),b,c);

    return a;
}

/**
 * @brief Get the shortest string
 * @param Vector of input strings
 * @return Longest string
 */
std::string utf8fcns::shortest(const std::vector<std::string> &strs)
{
    std::vectro::const_iterator it, it0;
    size_t len = 0;
    for (strs.begin(); it!=strs.end(); ++it)
    {
        size_t l = utf8fcns::len(*it);
        if (l<=len)
        {
            len = l;
            it0 = it;
        }
    }

    return *it0;
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
    boost::u32regex prefix_exp = boost::make_u32regex(prefix_pattern);
    u32regex_replace(x.start, x.end, prefix_exp, "",
                        boost::regex::perl|boost::regex::icase|boost::regesx::format_first_only);

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
    boost::u32regex prefix_exp = boost::make_u32regex(prefix_pattern);

    boost::match_results<std::string::const_iterator> what;
    if (u32regex_search(x.start, x.end, what, prefix_exp, boost::regex::perl|boost::regex::icase))
    {
        // format first word of x: first letter, upper-case, rest lower-case
        what.format("$', $1");
    }

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

}
