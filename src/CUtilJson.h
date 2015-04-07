#pragma once

#include <string>
#include <iostream>

#include <jansson.h>

#include "IDatabase.h"

/** Abstract base Database class with CD info stored in JSON format
 */
class CUtilJson
{
public:
    /** Constructor.
     */
    CUtilJson(const std::string &data="");
    CUtilJson(const CUtilJson &src);

    /** Destructor
     */
    virtual ~CUtilJson();

    /** Debug function. Prints full-struct of JSON object for a release
     *
     *  @param[in] Record index to print'
     *  @param[in] Output stream to print the object (default: cout)
     */
    virtual void PrintJSON(std::ostream &os=std::cout) const { PrintJSON(data, os); }

    // functions to get value of a requested key off the root
    bool FindBool(const std::string &key, bool &val) const { return FindBool(data, key, val); }
    bool FindInt(const std::string &key, json_int_t &val) const { return FindInt(data, key, val); }
    bool FindDouble(const std::string &key, double &val) const { return FindDouble(data, key, val); }
    bool FindString(const std::string &key, std::string &val) const { return FindString(data, key, val); }
    bool FindObject(const std::string &key, json_t* &val) const { return FindObject(data, key, val); }
    bool FindArray(const std::string &key, json_t* &val) const { return FindArray(data, key, val); }

    /**
     * @brief CompareString_ compare string root key value to the given string
     * @param[in] JSON element key word
     * @param[in] String object to compare to (the comparing string)
     * @return     A signed integral indicating the relation between the strings: 0 if equal,
     *             <0 if key is invalid or not a string key or key string is longer than
     *             the comparing string, or >0 if first key string is shorter than the
     *             comparing string
     */
    int CompareString(const std::string &key, const std::string &str) const { return CompareString(data, key, str); }

    json_t *data;
    json_error_t error;

    /** Debug function. Prints full-struct of JSON object for a release
     *
     *  @param[in] Record index to print'
     *  @param[in] Output stream to print the object (default: cout)
     */
    static void PrintJSON(const json_t* obj, std::ostream &os=std::cout);

    // generic static functions to get value of a requested key
    static bool FindBool(const json_t* obj, const std::string &key, bool &val);
    static bool FindInt(const json_t* obj, const std::string &key, json_int_t &val);
    static bool FindDouble(const json_t* obj, const std::string &key, double &val);
    static bool FindString(const json_t* obj, const std::string &key, std::string &val);
    static bool FindObject(const json_t* obj, const std::string &key, json_t* &val);
    static bool FindArray(const json_t* obj, const std::string &key, json_t* &val);

    /**
     * @brief CompareString_ compare string key value to the given string
     * @param[in] Pointer to a JSON object
     * @param[in] JSON element key word
     * @param[in] String object to compare to (the comparing string)
     * @return     A signed integral indicating the relation between the strings: 0 if equal,
     *             <0 if key is invalid or not a string key or key string is longer than
     *             the comparing string, or >0 if first key string is shorter than the
     *             comparing string
     */
    static int CompareString(const json_t* obj, const std::string &key, const std::string &str);

private:
    static void PrintJSON_(std::ostream &os, const char *key, json_t *value, const std::string pre);
    static void PrintJSON_value_(std::ostream &os, json_t *value, const std::string pre);
};
