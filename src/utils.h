#pragma once

#include <string>

/**
 * @brief Remove non-digit characters from UPC barcode
 * @param[in] UPC string returned from a database
 */
void cleanup_upc(std::string &upc);

