#include "CFileNameGenerator.h"

#include <algorithm>
#include <ctype.h>
#include <boost/regex.hpp>
#include <boost/regex/icu.hpp>
#include <unicode/unistr.h>

/**
 * @brief Constructor
 * @param[in] file naming scheme string
 * @param[in] file format
 */
CFileNameGenerator::CFileNameGenerator(const std::string &schemein, const OutputFileFormat &fmtin)
    : scheme(schemein), fmt(fmtin)
{}

/**
 * @brief generate a file name from a cuesheet object
 * @param[in] populated cuesheet
 * @return generated file name string
 */
std::string CFileNameGenerator::operator()(const SCueSheet &cuesheet) const
{
    ParserOutput result = parser(cuesheet);
    return result.str;
}

/**
 * @brief (recursive) working function for operator()
 * @param[in] populated cuesheet
 * @param[in] starting position on the scheme
 * @param[in] terminating character
 * @return generated file name string
 */
CFileNameGenerator::ParserOutput CFileNameGenerator::parser(const SCueSheet &cuesheet, size_t pos0,
                                        const std::string &termch) const
{
    ParserOutput rval;

    std::string word;
    ParserOutput subexpr;
    size_t len;

    // loop until end of scheme or come across the terminating character
    for (size_t pos = scheme.find_first_of("[%'$",pos0);
         pos!=scheme.npos && termch.npos==termch.find_first_not_of(scheme[pos]);
         pos = scheme.find_first_of("[%'$",pos0))
    {
        // add up-to the found character
        rval.str += scheme.substr(pos0, pos-pos0);

        // update the reference position
        pos0 = pos + 1;

        switch (scheme[pos])
        {
        case '[': // conditional section
            // parse the conditional section
            subexpr = parser(cuesheet,pos0,"]");
            if (pos==scheme.npos)
                throw(std::invalid_argument("Invalid Filename Scheme: Closing bracket of conditional section not found."));

            // if conditional section returned the truth value true, add its string
            if (subexpr.tf)
            {
                rval.str += subexpr.str;
                rval.tf = true;
            }
            pos = subexpr.end;
            break;
        case '\'': // quoted text found
            pos = scheme.find_first_of('\'',pos0);
            if (pos==pos0) // two consecutive quotes
                rval.str += '\'';
            else // if not closed, assume lasts until the end of scheme string
                rval.str += scheme.substr(pos0,pos-pos0);
            break;
        case '%': // variable found
            // find the end of variable name
            pos = scheme.find_first_of('%',pos0);
            if (pos==scheme.npos)
                throw(std::invalid_argument("Invalid Filename Scheme: variable name not closed."));

            // record the current string length
            len = rval.str.size();

            // grab the variable name
            word = scheme.substr(pos0,pos-pos0);

            // convert variable name to lower case (varaible names are all ascii)
            std::transform(word.begin(), word.end(), word.begin(), ::toupper);

            // convert metadata aliases to cuesheet field names
            if (word.compare("DISCNUMBER")==0) word = "DISC";
            else if (word.compare("TOTALDISC")==0) word = "DISCS";

            // get the cuesheet field values
            if (word.compare("ALBUM")==0)
            {
                rval.str += cuesheet.Title;
            }
            else
            {
                // check for artist/performer/songwriter
                bool isart=false, isperf=false, iscomp=false;
                if ((isart=(word.compare(0,6,"ARTIST")==0))
                        || (isperf=(word.compare(0,9,"PERFORMER")==0))
                        || (iscomp=(word.compare(0,10,"SONGWRITER")==0)))
                {
                    // get the artist name vector
                    SCueArtists names;
                    if (isart||isperf) names = cuesheet.Performer;
                    if (iscomp||(isart&&names.empty())) names = cuesheet.Songwriter;

                    // get the artist option(s)
                    bool firstonly=false, lastname=false, firstinitial=false;
                    size_t vpos = word.find_first_of(" ");
                    while (vpos!=word.npos)
                    {
                        size_t vlen;
                        size_t vpos0 = vpos+1;
                        vpos = word.find_first_of(" ",vpos0);

                        if (vpos!=word.npos) vlen = (vpos++)-vpos0;
                        else vlen = word.size()-vpos0;

                        if (word.compare(vpos0, vlen, "first")==0)
                            firstonly = true;
                        else if (word.compare(vpos0, vlen, "lastname")==0)
                            lastname = true;
                        else if (word.compare(vpos0, vlen, "firstinitial")==0)
                            firstinitial = true;
                    }

                    // form name
                    rval.str += FormName_(names, firstonly, 2*firstinitial + lastname);
                }
                else // if not artist, check metadata in REMs
                {
                    rval.str = FindRem_(cuesheet,word);
                }
            }

            // if newly added variable value was non-empty, return true
            if (rval.str.size()!=len) rval.tf = true;
            break;
        default : // case '$': // functions
            // find the end of function name
            pos = scheme.find_first_of('(',pos0);
            if (pos==scheme.npos)
                throw(std::invalid_argument("Invalid Filename Scheme: function name not closed."));

            // grab the variable name
            word = scheme.substr(pos0,pos-pos0);

            // convert variable name to lower case (varaible names are all ascii)
            std::transform(word.begin(), word.end(), word.begin(), ::toupper);

            pos0 = pos + 1;
            if (word.compare("IF")==0)  // $IF(COND,THEN) or $IF(COND,THEN,ELSE)
            {
                bool iftf;

                // parse the conditional section
                subexpr = parser(cuesheet,pos0,",");
                if (pos==scheme.npos)
                    throw(std::invalid_argument("Invalid Filename Scheme: IF COND not closed."));
                iftf = subexpr.tf;

                // adjust pos0 to the beginining of the next clause
                pos0 = subexpr.end + 1;

                // get then clause
                subexpr = parser(cuesheet,pos0,",)");
                if (pos==scheme.npos)
                    throw(std::invalid_argument("Invalid Filename Scheme: IF THEN not closed."));

                // if COND is true, copy THEN clause
                if (iftf)
                {
                    rval.str += subexpr.str;
                    rval.tf = rval.tf || subexpr.tf;
                }

                // if ELSE clause is available, process it
                if (scheme[subexpr.end]==',') // else clause exists
                {
                    pos0 = subexpr.end + 1;
                    subexpr = parser(cuesheet,pos0,")");
                    if (pos==scheme.npos)
                        throw(std::invalid_argument("Invalid Filename Scheme: IF ELSE not closed."));

                    // if COND is false, copy ELSE clause
                    if (!iftf)
                    {
                        rval.str += subexpr.str;
                        rval.tf = rval.tf || subexpr.tf;
                    }
                }
            }
            else if (word.compare("IF2")==0 ||  // $IF2(A,ELSE)
                     word.compare("IF3")==0)    // $IF3(A1,A2,...,AN,ELSE)
            {
                bool iftf=false;
                size_t counter = 0;

                // loop until closing expression is found
                while (scheme[pos0]!=')')
                {
                    // parse the conditional section
                    subexpr = parser(cuesheet,pos0,",)");
                    if (pos==scheme.npos)
                        throw(std::invalid_argument("Invalid Filename Scheme: IF2/IF3 not closed."));

                    // update iftf only if it has not been set true
                    if (!iftf)
                    {
                        iftf = subexpr.tf;
                        if (iftf) rval.str += subexpr.str;
                    }

                    // adjust pos0 to the beginining of the next clause
                    pos0 = subexpr.end + 1;

                    // count
                    counter++;
                }

                if (counter<2)
                    throw(std::invalid_argument("Invalid Filename Scheme: IF2/IF3 not enough arguments."));

                // if all (incl. ELSE clause) are false, get the last one (ELSE)
                if (!iftf) rval.str += subexpr.str;
            }
            else if (word.compare("AND")==0)    // $AND(...)
            {
                rval.tf = true;

                if (scheme[pos0]==')')
                    throw(std::invalid_argument("Invalid Filename Scheme: AND must have at least one argument."));

                // loop until closing expression is found
                while (scheme[pos0]!=')')
                {
                    // parse the conditional section
                    subexpr = parser(cuesheet,pos0,",)");
                    if (pos==scheme.npos)
                        throw(std::invalid_argument("Invalid Filename Scheme: AND not closed."));

                    // update iftf only if it has not been set true
                    if (!subexpr.tf) rval.tf = false;

                    // adjust pos0 to the beginining of the next clause
                    pos0 = subexpr.end + 1;
                }
            }
            else if (word.compare("OR")==0)    // $OR(...)
            {
                if (scheme[pos0]==')')
                    throw(std::invalid_argument("Invalid Filename Scheme: OR must have at least one argument."));

                // loop until closing expression is found
                while (scheme[pos0]!=')')
                {
                    // parse the conditional section
                    subexpr = parser(cuesheet,pos0,",)");
                    if (pos==scheme.npos)
                        throw(std::invalid_argument("Invalid Filename Scheme: OR not closed."));

                    // update iftf only if it has not been set true
                    if (subexpr.tf) rval.tf = true;

                    // adjust pos0 to the beginining of the next clause
                    pos0 = subexpr.end + 1;
                }
            }
            else if (word.compare("NOT")==0)    // $NOT(X)
            {
                if (scheme[pos0]==')')
                    throw(std::invalid_argument("Invalid Filename Scheme: OR must have at least one argument."));

                // parse the conditional section
                subexpr = parser(cuesheet,pos0,")");
                if (pos==scheme.npos)
                    throw(std::invalid_argument("Invalid Filename Scheme: NOT is not properly closed."));

                rval.tf = !subexpr.tf;
            }
            else if (word.compare("XOR")==0)    // $XOR(...)
            {
                if (scheme[pos0]==')')
                    throw(std::invalid_argument("Invalid Filename Scheme: OR must have at least one argument."));

                // loop until closing expression is found
                while (scheme[pos0]!=')')
                {
                    // parse the conditional section
                    subexpr = parser(cuesheet,pos0,",)");
                    if (pos==scheme.npos)
                        throw(std::invalid_argument("Invalid Filename Scheme: XOR not closed."));

                    // update iftf only if it has not been set true
                    if (subexpr.tf) rval.tf = !rval.tf;

                    // adjust pos0 to the beginining of the next clause
                    pos0 = subexpr.end + 1;
                }

            }
            else if (word.compare("STRCMP")==0 ||   // $STRCMP(S1,S2)
                     word.compare("STRCMPI")==0)    // $STRCMPI(S1,S2)
            {
                ParserOutput subexpr2;

                if (scheme[pos0]==')')
                    throw(std::invalid_argument("Invalid Filename Scheme: STRCMP must have two arguments."));

                // parse the conditional section
                subexpr = parser(cuesheet,pos0,",");
                if (pos==scheme.npos)
                    throw(std::invalid_argument("Invalid Filename Scheme: STRCMP must have two arguments."));

                // adjust pos0 to the beginining of the next clause
                pos0 = subexpr.end + 1;

                subexpr2 = parser(cuesheet,pos0,")");

                if (scheme[pos0]==')')
                    throw(std::invalid_argument("Invalid Filename Scheme: STRCMP must have two arguments."));

                if (word.size()==6) // $STRCMP(S1,S2)
                {
                    rval.tf = subexpr.str.compare(subexpr2.str)==0;
                }
                else // $STRCMPI(S1,S2)
                {
                    icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(subexpr.str);
                    rval.tf = ustr.caseCompare(icu::UnicodeString::fromUTF8(subexpr2.str),
                                               U_FOLD_CASE_DEFAULT);
                }
            }
            else if (word.compare("CAPS")==0) // $CAPS(X)
            {
                if (scheme[pos0]==')')
                    throw(std::invalid_argument("Invalid Filename Scheme: CAPS must have two arguments."));

                // parse the conditional section
                subexpr = parser(cuesheet,pos0,")");
                if (pos==scheme.npos)
                    throw(std::invalid_argument("Invalid Filename Scheme: CAPS must have two arguments."));

            }
        }
    }

    if (termch.empty()) // main call
    {
        // add the remainder of the characters in the scheme
        rval.str += scheme.substr(pos0);

        // add the extension
        rval.str += to_ext(fmt);

        rval.end = scheme.npos;
    }
    else // sub call
    {
        rval.end = pos0;
    }

    return rval;
}

/**
 * @brief Find REM field value
 * @param[in] populated cuesheet
 * @param[in] REM field name (first word following REM)
 * @return REM field value
 */
std::string CFileNameGenerator::FindRem_(const SCueSheet &cuesheet, const std::string &rem_type) const
{
    std::string pattern = rem_type + "\\s+(.*)"; // NAME XXXXX
    boost::u32regex exp = boost::make_u32regex(pattern);

    std::string rval;
    bool still_looking = true;
    boost::smatch what;

    // look through REMS field for the matching key
    for (std::vector<std::string>::const_iterator it = cuesheet.Rems.begin();
         still_looking && it !=cuesheet.Rems.end();
         ++it)
    {
        // get the first word
        if (boost::u32regex_match(*it, what, exp))
        {
            still_looking = false;
            rval = what[1];
        }
    }

    return rval;
}

/**
 * @brief Generate name string according to the specified format option
 * @param[in] list of artists to be concatenated
 * @param[in] number of artists to include (<=0 to include all)
 * @param[in] if individual artist, 0-full, 1-initials+last, 2 last only
 * @return formatted name string
 */
std::string CFileNameGenerator::FormName_(const SCueArtists &artists, const int numlist, const int option) const
{
    std::string rval;
    SCueArtists::const_iterator artist, begin, end;

    // set iterator start & end points
    bool allartists = (numlist<0 || numlist>(int)artists.size()); // all artists

    if (allartists)
    {
        begin = artists.begin();
        end = artists.end();
    }
    else // only the (numlist)-th artist (if 0, only the first)
    {
        begin = artists.begin()+numlist;
        end = begin+1;
    }

    // for each artist, parse the name followed by the joining string
    for (artist= begin; artist!=end; ++artist)
    {
        if (artist->type==SCueArtistType::PERSON)
        {
            switch (option)
            {
            case 1: // first middle initials + last name
            case 3:
                rval += InitialsPlusLastName_(artist->name);
                break;
            case 2: // last name only
                rval += LastName_(artist->name);
                break;
            default:
                rval += artist->name;
            }
        }
        else
        {
            rval += artist->name;
        }

        if (allartists) rval += artist->joiner;
    }

    return rval;
}

/**
 * @brief Extract last name
 * @param[in] fullname string
 * @param[out] (Optional) iterator at the first non-word character preceeding
 *             the last name (use this as the end point of the initials search)
 * @return string containing just last name
 */
std::string CFileNameGenerator::LastName_(const std::string &fullname,
                                          std::string::const_iterator *start)
{
    static const std::string pattern("\\s*?([\\S\\D]+?)[\\W\\d]*?$"); // last-name
    static const boost::u32regex exp = boost::make_u32regex(pattern);

    std::string rval;
    boost::match_results<std::string::const_iterator> what;
    if (u32regex_search(fullname.begin(), fullname.end(), what, exp))
    {
        rval = what[1];
        if (start) *start = what[0].first;
    }
    return rval;
}

/**
 * @brief Extract initials + last name
 * @param[in] fullname string
 * @return string containing initials (no space between) + lastname
 */
std::string CFileNameGenerator::InitialsPlusLastName_(const std::string &fullname)
{
    static const std::string pattern("(\\w)\\w*(\\W*)"); // initials
    static const boost::u32regex exp = boost::make_u32regex(pattern);
    static const boost::u32regex_iterator<std::string::const_iterator> iend;

    std::string rval;

    // retrieve the last name
    std::string::const_iterator end;
    std::string lastname = LastName_(fullname,&end);

    // if successful and more characters are available, get initials
    if (lastname.size() && end!=fullname.begin())
    {
        int skip = 0; // all bracketed/quoted words will be ignored

        for (boost::u32regex_iterator<std::string::const_iterator> i(boost::make_u32regex_iterator(fullname, exp));
             i != iend && (*i)[0].second>=end;
             ++i)
        {
            if (!skip) rval += (*i)[1]; // add initial if outside of brackets/quotes

            // check for brackets and quotations
            CheckBrackets_((*i)[3].first,(*i)[3].second,skip);
        }
        rval += " " + lastname;
    }

    return rval;
}

void CFileNameGenerator::CheckBrackets_(const std::string::const_iterator begin,
                                        const std::string::const_iterator end,
                                        int &skip)
{
    static const std::string pattern("\\s*([[:Ps:][:Pi:]])|([[:Pe:][:Pf:]])\\s*"); // a bracket
    static const boost::u32regex exp = boost::make_u32regex(pattern);
    static const boost::u32regex_iterator<std::string::const_iterator> iend;

    std::string str(begin,end);

    for (boost::u32regex_iterator<std::string::const_iterator> i(boost::make_u32regex_iterator(str, exp));
         i != iend;
         ++i)
    {
        if ((*i)[1].matched) ++skip;
        else --skip;
    }
}
