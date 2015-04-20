#include "CFileNameGenerator.h"

#include <algorithm>

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
std::string CFileNameGenerator::operator()(const SCueSheet &cuesheet)
{

    std::string rval;
    rval.reserve(256);

    std::stack<std::string> stack;
    bool cond;

    size_t pos0 = 0;
    for (size_t pos = scheme.find_first_of("[]%'$");
         pos!=scheme.npos;
         pos = scheme.find_first_of("[]%'$",pos0))
    {
        // add up-to the found character
        rval += scheme.substr(pos0, pos-pos0);

        if (scheme[pos]=='[') // conditional section
        {

        }
        else if (scheme[pos]=='\'') // quoted text found
        {
            pos0 = pos + 1;
            pos = scheme.find_first_of('\'',pos0);
            if (pos==scheme.npos)
                throw(std::invalid_argument("Invalid Filename Scheme: quotation not closed."));
            else if (pos==pos0) // two consecutive quotes
                rval += '\'';
            else
                rval += scheme.substr(pos0,pos-pos0);
        }
        else // variable found
        {
            pos0 = pos + 1;
            pos = scheme.find_first_of('%',pos0);
            if (pos==scheme.npos)
                throw(std::invalid_argument("Invalid Filename Scheme: variable name not closed."));

            // grab the variable name
            std::string var = scheme.substr(pos0,pos-pos0);

            // convert variable name to lower case (varaible names are all ascii)
            std::transform(data.begin(), data.end(), data.begin(), ::toupper);

            // revert metadata aliases
            if (var.compare("DISCNUMBER")==0) var = "DISC";
            else if (var.compare("TOTALDISC")==0) var = "DISCS";

            if (var.compare("ALBUM")==0)
            {
                rval += cuesheet.Title;
            }
            else // album title -or- metadata in REM fields
            {
                bool isart=false, isperf=false, iscomp=false;
                if ((isart=(var.compare(0,6,"ARTIST")==0))
                        || (isperf=(var.compare(0,9,"PERFORMER")==0))
                        || (iscomp=(var.compare(0,10,"SONGWRITER")==0)))
                {
                    // get the artist name vector
                    CCueArtists names;
                    if (isart||isperf) names = cuesheet.Performer;
                    if (iscomp||(isart&&names.empty())) names = cuesheet.SongWriter;

                    // get the artist option(s)
                    bool firstonly=false, lastname=false, firstinitial=false;
                    size_t vpos = var.find_first_of(" ");
                    while (vpos!=var.npos)
                    {
                        size_t vlen;
                        size_t vpos0 = vpos+1;
                        vpos = var.find_first_of(" ",vpos0);

                        if (vpos!=var.npos) vlen = (vpos++)-vpos0;
                        else vlen = var.size()-vpos0;

                        if (var.compare(vpos0, vlen, "first")==0)
                            firstonly = true;
                        else if (var.compare(vpos0, vlen, "lastname")==0)
                            lastname = true;
                        else if (var.compare(vpos0, vlen, "firstinitial")==0)
                            firstinitial = true;
                    }

                    // form name
                    rval += FormName_(names, firstonly, 2*firstinitial + lastname);
                }
                else // if not artist, check metadata in REMs
                {
                    rval += FindRem_(cuesheet,var);
                }
            }
//* - %artist%
//* - %performer%
//* - %songwriter%
//* - %artist first%, %performer first%, %songwriter first%
//* - %artist lastname%, %performer lastname%, %songwriter lastname%
//* - %album%

            rval += ;
        }
    }

    // add the remainder of the characters in the scheme
    rval += scheme.substr(pos0);

    // add the extension
    rval += to_ext(fmt);

    return rval;

//    std::transform(data.begin(), data.end(), data.begin(), ::tolower);
}

/**
 * @brief Find REM field value
 * @param[in] populated cuesheet
 * @param[in] REM field name (first word following REM)
 * @return REM field value
 */
std::string CFileNameGenerator::FindRem_(const SCueSheet &cuesheet, const std::string &rem_type)
{
    // get metadata value as string
    switch (type)
    {
    case AlbumRemFieldType::DBINFO:
        remval = to_string(rdb.GetDatabaseType()) + " " + rdb.ReleaseId(recid);
        break;
    case AlbumRemFieldType::GENRE:
        remval = rdb.Genre(recid);
        break;
    case AlbumRemFieldType::DATE:
        remval = rdb.Date(recid);
        break;
    case AlbumRemFieldType::COUNTRY:
        remval = rdb.Country(recid);
        break;
    case AlbumRemFieldType::UPC:
        remval = rdb.AlbumUPC(recid);
        break;
    case AlbumRemFieldType::LABEL:
        remval = rdb.AlbumLabel(recid);
        break;
    case AlbumRemFieldType::CATNO:
        remval = rdb.AlbumCatNo(recid);
        break;
    case AlbumRemFieldType::DISC:
        remval.clear();
        if (rdb.TotalDiscs()>1)
        {
            int no = rdb.DiscNumber();
            if (no>0) remval = std::to_string(no);
        }
        break;
    case AlbumRemFieldType::DISCS:
        remval.clear();
        int no = rdb.TotalDiscs();
        if (no>1) remval = std::to_string(no);
        break;
    }

    // if metadata value is not empty, insert
    if (remval.size())
        rem = to_string(remfields[i]) + " " + remval;
}

/**
 * @brief Generate name string according to the specified format option
 * @param[in] list of artists to be concatenated
 * @param[in] number of artists to include (<=0 to include all)
 * @param[in] 0-full name, 1-first initial + last, 2 last only
 * @return formatted name string
 */
std::string CFileNameGenerator::FormName_(const SCueArtists &artists, const int numlist, const int option)
{
    std::string rval;

    return rval;
}
