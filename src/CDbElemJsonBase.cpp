#include "CDbJsonBase.h"

#include <stdexcept>

using std::endl;
using std::runtime_error;

/** Constructor.
 */
CDbJsonBase::CDbJsonBase() {}

/** Destructor
 */
CDbJsonBase::~CDbJsonBase()
{
    // clear all stored JSON objects
    std::vector<json_t*>::iterator it;
    for (it=Data.begin();it!=Data.end();it++) json_object_clear(*it);
}

void CDbJsonBase::ClearData_()
{
    // clear all stored JSON objects
    std::vector<json_t*>::iterator it;
    for (it=Data.begin();it!=Data.end();it++) json_object_clear(*it);

    // then empty the Data vector
    Data.clear();
    errors.clear();
}

json_t* CDbJsonBase::AppendRelease_(const std::string &data)
{
    // first create a dedicated error handling struct for the new release
    errors.emplace_back();

    json_t *root;

    // now load the data onto json object
    try
    {
        root = json_loadb(data.c_str(), data.size(), 0, &errors.back());
        if(!root) throw(std::runtime_error(errors.back().text));

        // push the new release record onto the vector
        Data.push_back(root);
    }
    catch (...)
    {
        errors.pop_back(); // failed to append new release, no need to keep its error struct
        throw; // rethrow the exception
    }
    return root;
}


// helper functions to get value of a requested key
bool CDbJsonBase::FindBool_(const json_t* obj, const std::string &key, bool &val)
{
    json_t * value = json_object_get(obj, key.c_str());

    if (value==NULL) return false; // key not found
    if (json_is_true(value)) val = true;
    else if (json_is_false(value)) val = false;
    else return false; // not bool
    return true;
}

bool CDbJsonBase::FindInt_(const json_t* obj, const std::string &key, json_int_t &val)
{
    json_t * value = json_object_get(obj, key.c_str());
    if (value==NULL) return false; // key not found
    if (json_is_integer(value)) val = json_integer_value(value);
    else return false; // not int
    return true;
}

bool CDbJsonBase::FindDouble_(const json_t* obj, const std::string &key, double &val)
{
    json_t * value = json_object_get(obj, key.c_str());
    if (value==NULL) return false; // key not found
    if (json_is_integer(value)) val = (double)json_integer_value(value);
    else if (json_is_real(value)) val =  json_real_value(value);
    else return false; // not real
    return true;
}

bool CDbJsonBase::FindString_(const json_t* obj, const std::string &key, std::string &val)
{
    json_t * value = json_object_get(obj, key.c_str());
    if (value==NULL) return false; // key not found
    if (json_is_string(value)) val = json_string_value(value);
    else if (json_is_null(value)) val = "";
    else return false; // not string (or null)
    return true;
}

/**
 * @brief CompareString_ compare string key value to the given string
 * @param[in] Pointer to a JSON object
 * @param[in] JSON element key word
 * @param[in] String object to compare to (the compared string)
 * @return     A signed integral indicating the relation between the strings: 0 if equal,
 *             <0 if key is invalid or not a string key or key string is longer than
 *             the compared string, or >0 if first key string is shorter than the
 *             compared string
 */
int CDbJsonBase::CompareString_(const json_t* obj, const std::string &key, const std::string &str)
{
    json_t * value = json_object_get(obj, key.c_str());
    if (value!=NULL && json_is_string(value)) return str.compare(json_string_value(value));
    return -1; // key doesn't exist or its value is not a string
}

bool CDbJsonBase::FindObject_(const json_t* obj, const std::string &key, json_t* & val)
{
    val = json_object_get(obj, key.c_str());
    return val!=NULL && json_is_object(val);
}

bool CDbJsonBase::FindArray_(const json_t* obj, const std::string &key, json_t* & val)
{
    val = json_object_get(obj, key.c_str());
    return val!=NULL && json_is_array(val);
}

void CDbJsonBase::PrintJSON(const int recnum, std::ostream &os) const
{
    // if invalid record number, do nothing
    if (recnum<0 || recnum>=(int)Data.size()) return;

    const char *key;
    json_t *value;
    json_object_foreach(Data[recnum], key, value)
    {
        PrintJSON_(os, key, value, "");
    }
}

void CDbJsonBase::PrintJSON_(std::ostream &os, const char *key, json_t *value, std::string pre)
{
    /* block of code that uses key and value */
    if (json_typeof(value)==JSON_ARRAY)
    {
        size_t index;
        json_t *array_val;
        json_array_foreach(value,index,array_val)
        {
            os << pre << key << "[" << index << "]";
            PrintJSON_value_(os, array_val, pre);
        }
    }
    else
    {
        os << pre << key;
        PrintJSON_value_(os, value, pre);
    }
}

void CDbJsonBase::PrintJSON_value_(std::ostream &os, json_t *value, const std::string pre)
{
    switch (json_typeof(value))
    {
    case JSON_OBJECT:

        os << endl;

        const char *obj_key;
        json_t *obj_val;

        json_object_foreach(value, obj_key, obj_val)
        {
            PrintJSON_(os, obj_key, obj_val, pre+" ");
        }

        break;
    case JSON_STRING:
        os << "(s): " << json_string_value(value) << endl;
        break;
    case JSON_INTEGER:
        os << "(i): " << json_integer_value(value) << endl;
        break;
    case JSON_REAL:
        os << "(r): " << json_real_value(value) << endl;
        break;
    case JSON_TRUE:
        os << "(b): " << "TRUE" << endl;
        break;
    case JSON_FALSE:
        os << "(b): " << "FALSE" << endl;
        break;
    case JSON_NULL:
        os << ": " << "(NULL)" << endl;
        break;
    default:
        os << ": " << "UNKNOWN" << endl;
    }
}
