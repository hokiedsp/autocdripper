#include "utils.h"

#include <cstring>
#include <algorithm>
#include <vector>
#include <stdexcept>

using std::vector;
using std::string;

/**
 * @brief Remove non-digit characters from UPC barcode
 * @param[in] UPC string returned from a database
 */
void cleanup_upc(std::string &upc)
{
    upc.erase(std::remove_if(upc.begin(),
                        upc.end(),
                        [](const char& s){ return NULL==strchr("0123456789",s); }),
              upc.end());
}

/**
 * @brief Express an integer as a Roman numeral.
 * @param Value must be between 1 and 3999
 * @return string containing Roman numeral
 * @throw std::out_of_range if val is not in (0,4000)
 */
std::string itoroman (int val)
{
    if (val<1 || val>=4000) throw(std::out_of_range("Roman numeral expression is limited to integers less than 4000."));

    string rval;
    rval.reserve(15); // largest # supported: 3888, MMMDCCCLXXXVIII

    for (int base = 1000; base; base/=10)
    {
        char roman_char;
        unsigned int digit = val/base;
        val = val%base;

        bool isnine = digit==9; // i.e., IX, XC, CM
        bool isfour = digit==4; // i.e., IV, XL, CD

        if (isnine || isfour) // "one" followed by "ten" or "five"
        {
            digit = 1;
        }
        else if (digit>4) // 5-8, "five" preceeding "ones"
        {
            if (base==1) rval += 'V';
            else if (base==10) rval += 'L';
            else rval += 'D';
            digit -= 5;
        }

        if (digit) // print "ones"
        {
            if (base==1) roman_char = 'I';
            else if (base==10) roman_char = 'X';
            else if (base==100) roman_char = 'C';
            else roman_char = 'M';

            for (;digit;digit--) rval +=roman_char;
        }

        // print "ten" or "five" the number is one shy of
        if (isnine)
        {
            if (base==1) rval += 'X';
            else if (base==10) rval += 'C';
            else rval += 'M';
        }
        else if (isfour)
        {
            if (base==1) rval += 'V';
            else if (base==10) rval += 'L';
            else rval += 'D';
        }
    }

    return rval;
}
