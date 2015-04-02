#include "CUtilJson.h"

#include <stdexcept>

using std::endl;
using std::runtime_error;

/** Constructor.
 */
CUtilJson::CUtilJson(const std::string &rawdata)
{
    if (rawdata.size())
    {
        // now load the data onto json object
        try
        {
            data = json_loadb(rawdata.c_str(), rawdata.size(), 0, &error);
            if(!data) throw(std::runtime_error(error.text));
        }
        catch (...)
        {
            throw; // rethrow the exception
        }
    }
    else
    {
        data = NULL;
    }
}

/** Destructor
 */
CUtilJson::~CUtilJson()
{
    json_object_clear(data);
}

// helper functions to get value of a requested key
bool CUtilJson::FindBool(const json_t* obj, const std::string &key, bool &val)
{
    json_t * value = json_object_get(obj, key.c_str());

    bool rval = value; // key found
    if (rval)
    {
        if (json_is_true(value)) val = true;
        else if (json_is_false(value)) val = false;
        else rval = false; // not bool
    }
    return rval;
}

bool CUtilJson::FindInt(const json_t* obj, const std::string &key, json_int_t &val)
{
    json_t * value = json_object_get(obj, key.c_str());
    bool rval = value; // key found
    if (rval)
    {
        if (json_is_integer(value)) val = json_integer_value(value);
        else rval = false; // not int
    }
    return rval;
}

bool CUtilJson::FindDouble(const json_t* obj, const std::string &key, double &val)
{
    json_t * value = json_object_get(obj, key.c_str());
    bool rval = value;
    if (rval) // key found
    {
        if (json_is_integer(value)) val = (double)json_integer_value(value);
        else if (json_is_real(value)) val =  json_real_value(value);
        else rval = false; // not real
    }
    return rval;
}

bool CUtilJson::FindString(const json_t* obj, const std::string &key, std::string &val)
{
    json_t * value = json_object_get(obj, key.c_str());
    bool rval = value;
    if (rval) // key found
    {
        if (json_is_string(value)) val = json_string_value(value);
        else if (json_is_null(value)) val = "";
        else rval = false; // not string (or null)
    }
    return rval;
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
int CUtilJson::CompareString(const json_t* obj, const std::string &key, const std::string &str)
{
    int rval = -1; // if remained, key doesn't exist or its value is not a string

    json_t * value = json_object_get(obj, key.c_str());
    if (value!=NULL && json_is_string(value)) rval = str.compare(json_string_value(value));

    return rval;
}

bool CUtilJson::FindObject(const json_t* obj, const std::string &key, json_t* & val)
{
    val = json_object_get(obj, key.c_str());
    return val!=NULL && json_is_object(val);
}

bool CUtilJson::FindArray(const json_t* obj, const std::string &key, json_t* & val)
{
    val = json_object_get(obj, key.c_str());
    return val!=NULL && json_is_array(val);
}

void CUtilJson::PrintJSON(const json_t* obj, std::ostream &os)
{
    const char *key;
    json_t *value;
    json_object_foreach(const_cast<json_t*>(obj), key, value)
    {
        PrintJSON_(os, key, value, "");
    }
}

void CUtilJson::PrintJSON_(std::ostream &os, const char *key, json_t *value, std::string pre)
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

void CUtilJson::PrintJSON_value_(std::ostream &os, json_t *value, const std::string pre)
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