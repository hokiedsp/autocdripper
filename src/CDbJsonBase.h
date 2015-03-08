#pragma once

#include <string>
#include <iostream>
#include <jassonn.h>

#include "IDatabase.h"

/** Abstract base Database class with CD info stored in JSON format
 */
class CDbJsonBase
{
public:
    /** Constructor.
     */
    CDbJsonBase();

    /** Destructor
     */
    virtual ~CDbJsonBase();

    /** Returns the number of matched records returned from the last Query() call.
     *
     *  @return    Number of matched records
     */
    virtual int NumberOfMatches() const { return Releases.size(); }

    /** Debug function. Prints full-struct of JSON object for a release
     *
     *  @param[in] Record index to print'
     *  @param[in] Output stream to print the object (default: cout)
     */
    virtual void PrintJSON(const int recnum, std::ostream &os=std::cout) const;

protected:
    std::deque<json_t*> Releases;

    // helper functions to get value of a requested key
    static bool FindBool_(const json_t* obj, const std::string &key);
    static json_int_t FindInt_(const json_t* obj, const std::string &key);
    static double FindDouble_(const json_t* obj, const std::string &key);
    static std::string FindString_(const json_t* obj, const std::string &key);
    static json_t* FindObject_(const json_t* obj, const std::string &key);
    static json_t* FindArray_(const json_t* obj, const std::string &key);

private:
    static void PrintJSON_value_(std::ostream &os, const json_t *value,std::string pre);
    static void PrintJSON_(std::ostream &os, const char *key, const json_t *value, std::string pre);

};
