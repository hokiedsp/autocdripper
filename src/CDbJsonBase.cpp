/** Constructor.
 */
CDbJsonBase::CDbJsonBase() {}

/** Destructor
 */
CDbJsonBase::~CDbJsonBase()
{
    // clear all stored JSON objects
    std::deque<json_t*>::iterator it;
    for (it=Releases.begin();it!=Releases.end();it++) json_object_clear(*it);
}

void CDbJsonBase::PrintJSON(const int recnum, std::ostream &os) const
{
    // if invalid record number, do nothing
    if (recnum<0 || recnum>=(int)Releases.size()) return;

    const char *key;
    json_t *value;
    json_object_foreach(Releases[recnum], key, value)
    {
        PrintJSON_(os, key,value, "");
    }
}

// helper functions to get value of a requested key
bool CDbJsonBase::FindBool_(const json_t* obj, const std::string &key)
{
    json_t * value = json_object_get(obj, key.c_str());
    if (value==NULL) throw(1); // key not found
    if (json_is_true(value)) return true;
    else if (json_is_false(value)) return false;
    else throw(1); // not bool
}

json_int_t CDbJsonBase::FindInt_(const json_t* obj, const std::string &key)
{
    json_t * value = json_object_get(obj, key.c_str());
    if (value==NULL) throw(1); // key not found
    if (json_is_integer(value)) return json_integer_value(value);
    else throw(1); // not int
}

double CDbJsonBase::FindDouble_(const json_t* obj, const std::string &key)
{
    json_t * value = json_object_get(obj, key.c_str());
    if (value==NULL) throw(1); // key not found
    if (json_is_integer(value)) return (double)json_integer_value(value);
    else if (json_is_real(value)) return json_real_value(value);
    else throw(1); // not real
}

std::string CDbJsonBase::FindString_(const json_t* obj, const std::string &key)
{
    json_t * value = json_object_get(obj, key.c_str());
    if (value==NULL) throw(1); // key not found
    if (json_is_string(value)) return json_string_value(value);
    else if (json_is_null(value)) return "";
    else throw(1); // not string (or null)
}

json_t* CDbJsonBase::FindObject_(const json_t* obj, const std::string &key)
{
    json_t * value = json_object_get(obj, key.c_str());
    if (value==NULL) throw(1); // key not found
    if (json_is_object(value)) return value;
    else throw(1); // not object
}

json_t* CDbJsonBase::FindArray_(const json_t* obj, const std::string &key)
{
    json_t * value = json_object_get(obj, key.c_str());
    if (value==NULL) throw(1); // key not found
    if (json_is_array(value)) return value;
    else throw(1); // not array
}

void CDbJsonBase::PrintJSON_(std::ostream &os, const char *key, const json_t *value, std::string pre)
{
    /* block of code that uses key and value */
    if (json_typeof(value)==JSON_ARRAY)
    {
        size_t index;
        json_t *array_val;
        json_array_foreach(value,index,array_val)
        {
            os << pre << key << "[" << index << "]";
            PrintJSON_value_(os, array_val,pre);
        }
    }
    else
    {
        os << pre << key;
        PrintJSON_value_(os, value,pre);
    }
}

void CDbJsonBase::PrintJSON_value_(std::ostream &os, json_t *value,std::string pre)
{
    switch (json_typeof(value))
    {
    case JSON_OBJECT:
        const char *obj_key;
        json_t *obj_val;
        os << endl;
        json_object_foreach(value, obj_key, obj_val)
        {
            PrintJSON_value_(os, obj_key,obj_val,pre+" ");
        }
        break;
    case JSON_STRING:
        os << ": " << json_string_value(value) << endl;
        break;
    case JSON_INTEGER:
        os << ": " << json_integer_value(value) << endl;
        break;
    case JSON_REAL:
        os << ": " << json_real_value(value) << endl;
        break;
    case JSON_TRUE:
        os << ": " << "TRUE" << endl;
        break;
    case JSON_FALSE:
        os << ": " << "FALSE" << endl;
        break;
    case JSON_NULL:
        os << ": " << "NULL" << endl;
        break;
    default:
        os << ": " << "UNKNOWN" << endl;
    }
}

