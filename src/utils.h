#pragma once

#include <string>

/**
 * @brief Remove non-digit characters from UPC barcode
 * @param[in] UPC string returned from a database
 */
void cleanup_upc(std::string &upc);

/**
 * @brief Express an integer as a Roman numeral.
 * @param Value must be between 1 and 3999
 * @return string containing Roman numeral
 * @throw std::out_of_range if val is not in (0,4000)
 */
std::string itoroman (int val);
