#pragma once

#include <string>
#include <iostream>

#include <jansson.h>

#include "IDatabase.h"

/** Abstract base Database class with CD info stored in JSON format
 */
class CDbElemJsonBase
{
public:
    /** Constructor.
     */
    CDbElemJsonBase(const std::string &data);

    /** Destructor
     */
    virtual ~CDbElemJsonBase();

    /** Debug function. Prints full-struct of JSON object for a release
     *
     *  @param[in] Record index to print'
     *  @param[in] Output stream to print the object (default: cout)
     */
    virtual void PrintJSON(std::ostream &os=std::cout) const;

protected:
    json_t data;
    json_error_t errors;

    /**
     * @brief AppendRelease_
     * @param data
     * @return
     */
    json_t* NewData_();
    void DeleteData_(jsont_t* data);

    // helper functions to get value of a requested key
    static bool FindBool_(const json_t* obj, const std::string &key, bool &val);
    static bool FindInt_(const json_t* obj, const std::string &key, json_int_t &val);
    static bool FindDouble_(const json_t* obj, const std::string &key, double &val);
    static bool FindString_(const json_t* obj, const std::string &key, std::string &val);
    static bool FindObject_(const json_t* obj, const std::string &key, json_t* &val);
    static bool FindArray_(const json_t* obj, const std::string &key, json_t* &val);

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
    static int CompareString_(const json_t* obj, const std::string &key, const std::string &str);

private:
    std::vector<json_t*> Data;  // keep it private

    static void PrintJSON_(std::ostream &os, const char *key, json_t *value, const std::string pre);
    static void PrintJSON_value_(std::ostream &os, json_t *value, const std::string pre);

};
