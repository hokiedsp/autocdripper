#include "CFileNameGenerator.h"

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

            std::string var = scheme.substr(pos0,pos-pos0);

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
