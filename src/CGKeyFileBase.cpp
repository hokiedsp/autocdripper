#include "CGKeyFileBase.h"

#include <memory>

#include <iostream>

using std::string;
using std::runtime_error;

CGKeyFileBase::CGKeyFileBase(const std::string &filename)
    : file_name(filename)
{
    GError  *error = NULL;

    // create a new CONF file
    key_file = g_key_file_new();

    // try to open the file
    if (!g_key_file_load_from_file(key_file,filename.c_str(),G_KEY_FILE_KEEP_COMMENTS,&error))
    {
        // File does not exist, release current key_file object and let the derived class create a new one
        if(error->code == G_FILE_ERROR_NOENT || error->code==G_KEY_FILE_ERROR_NOT_FOUND)
        {
            g_key_file_free(key_file);
            key_file = NULL;
        }
        else // otherwise throw the error
        {
            throw(runtime_error(error->message));
        }
    }
}

CGKeyFileBase::~CGKeyFileBase()
{
    g_key_file_free(key_file);
}

void CGKeyFileBase::Save()
{
    GError  *error = NULL;
    if (!g_key_file_save_to_file(key_file, file_name.c_str(), &error))
        throw(runtime_error(error->message));
}

/**
 * @brief Create a new bool key with default value and key description
 * @param[in] a group name
 * @param[in[ a key
 * @param[in] the default value of the the key
 * @param[in] description of the key
 */
void CGKeyFileBase::CreateBooleanKey(const std::string &group, const std::string &key,
                                     const bool defval, const std::string &description)
{
    string comment;
    comment.reserve(1024);

    // set the key value
    g_key_file_set_boolean(key_file, group.c_str(), key.c_str(), defval);

    // create its comment
    comment = "Key: " + group + "::" + key + " (boolean)\n";
    comment += "Default: ";
    if (defval) comment += "true"; else comment += "false";
    comment += '\n';
    InsertDescription_(comment,description);

    // set the comment
    g_key_file_set_comment (key_file, group.c_str(), key.c_str(), comment.c_str(), NULL);
}

/**
 * @brief Create a new integer key with default value and key description
 * @param[in] a group name
 * @param[in[ a key
 * @param[in] the default value of the the key
 * @param[in] description of the key
 */
void CGKeyFileBase::CreateIntegerKey(const std::string &group, const std::string &key,
                                     const int defval, const std::string &description)
{
    string comment;
    comment.reserve(1024);

    // set the key value
    g_key_file_set_integer(key_file, group.c_str(), key.c_str(), defval);

    // create its comment
    comment = "Key: " + group + "::" + key + " (integer)\n";
    comment += "Default: " + std::to_string(defval) + '\n';
    InsertDescription_(comment,description);

    // set the comment
    g_key_file_set_comment (key_file, group.c_str(), key.c_str(), comment.c_str(), NULL);
}

/**
 * @brief Create a new string key with default value and key description
 * @param[in] a group name
 * @param[in[ a key
 * @param[in] the default value of the the key
 * @param[in] description of the key
 */
void CGKeyFileBase::CreateStringKey(const std::string &group, const std::string &key,
                                    const std::string defval,
                                    const std::string &description)
{
    string comment;
    comment.reserve(1024);

    // set the key value
    g_key_file_set_string (key_file, group.c_str(), key.c_str(), defval.c_str());

    // create its comment
    comment = "Key: " + group + "::" + key + " (string)\n";
    comment += "Default: " + defval + '\n';
    InsertDescription_(comment,description);

    // set the comment
    g_key_file_set_comment (key_file, group.c_str(), key.c_str(), comment.c_str(), NULL);
}

/**
 * @brief Create a new string key with default value and key description
 * @param[in] a group name
 * @param[in[ a key
 * @param[in] possible key values
 * @param[in] the default value of the the key
 * @param[in] description of the key
 */
void CGKeyFileBase::CreateStringKey(const std::string &group, const std::string &key,
                                    const std::vector<std::string> optvals,
                                    const std::string defval,
                                    const std::string &description)
{
    string comment;
    comment.reserve(1024);

    // set the key value
    g_key_file_set_string (key_file, group.c_str(), key.c_str(), defval.c_str());

    // create its comment
    comment = "Key: " + group + "::" + key + " (string)\n";

    if (optvals.size())
    {
        std::vector<string>::const_iterator it;
        it = optvals.begin();

        comment += "Options: ";
        comment += (*it++).c_str();
        while (it!=optvals.end())
        {
            comment += ", ";
            comment += (*it++).c_str();
        }
        comment += '\n';
    }
    comment += "Default: " + defval + '\n';
    InsertDescription_(comment,description);

    // set the comment
    g_key_file_set_comment (key_file, group.c_str(), key.c_str(), comment.c_str(), NULL);
}

/**
 * @brief Create a new string list key with default value and key description
 * @param[in] a group name
 * @param[in[ a key
 * @param[in] possible key values
 * @param[in] the default value of the the key
 * @param[in] description of the key
 */
void CGKeyFileBase::CreateStringListKey(const std::string &group, const std::string &key,
                                        const std::vector<std::string> optvals,
                                        const std::vector<std::string> defval,
                                        const std::string &description)
{
    std::vector<string>::const_iterator it;
    string comment;
    comment.reserve(1024);

    // convert vector to char**
    std::unique_ptr<const char *> charlist(new const char* [defval.size()]);
    const char **ptr = charlist.get();
    for(it = defval.begin();it!=defval.end();it++)
        *(ptr++) = (*it).c_str();

    // set the key value
    g_key_file_set_string_list(key_file, group.c_str(), key.c_str(), charlist.get(), defval.size());

    // create its comment
    comment = "Key: " + group + "::" + key + " (string list)\n";
    if (optvals.size())
    {
        it = optvals.begin();
        comment += "Possible String Values: ";
        comment += (*it++).c_str();
        while (it!=optvals.end())
        {
            comment += ", ";
            comment += (*it++).c_str();
        }
        comment += '\n';
    }

    comment += "Default: ";
    if (defval.size())
    {
        it = defval.begin();
        comment += (*it++).c_str();
        while (it!=defval.end())
        {
            comment += ';';
            comment += (*it++).c_str();
        }
        comment += '\n';
    }

    InsertDescription_(comment,description);

    // set the comment
    g_key_file_set_comment (key_file, group.c_str(), key.c_str(), comment.c_str(), NULL);
}

/**
 * @brief Word-wrap description text and insert it to the comment text
 * @param[out] key file comment text, description will be appended
 * @param[in] description text without word-warpping.
 */
void CGKeyFileBase::InsertDescription_(std::string &comment, std::string text)
{
    const char spacechars[] = " \t\n\v\f\r";
    const size_t width = 80;
    bool hasendl;

    if (text.empty()) return;

    comment += "Descriptions: "; //14 character indentation

    // mark true if at least one endl exists in text
    hasendl = text.size()>width && text.find_first_of('\n');

    while (text.size()>width)
    {
        string::size_type spacePos;

        // check for the last white space within the width
        spacePos = text.find_last_of(spacechars, width-1);

        if (spacePos == string::npos)
        {   // if no space within width is found, find the next space
            spacePos = text.find_first_of(spacechars);
            if (spacePos==string::npos) // single word text
            {
                comment += text;
                return;
            }
        }
        else if (hasendl)
        {
            string::size_type endlPos;

            // if white space is found, make sure there is no new line in this span
            endlPos = text.find_first_of('\n');
            if (endlPos!=string::npos && endlPos<spacePos)
                spacePos = endlPos;
        }

        // append the text to the comment
        comment += text.substr(0,spacePos);

        // erase the appended characters and the subsequent whitespace from text
        text.erase(0,spacePos+1);

        // if more text follows, insert new line and indentation to comment
        if (text.size()) comment += "\n              ";
    }

    // append the remaining text
    comment += text;
}

////////////////////////

/**
 * @brief Get the value of a boolean key. If key does not exist, its default value is returned
 * @param[in] a group name
 * @param[in] a key
 * @param[in] default value
 * @return retrieved key value or default value if key does not exist
 * @throw runtime_error if the key is not boolean
 */
bool CGKeyFileBase::GetBooleanKey(const std::string &group, const std::string &key, bool rval)
{
    GError *err = NULL;
    bool val = g_key_file_get_boolean(key_file, group.c_str(), key.c_str(), &err);
    if (!err)
        rval = val;
    else if ((err->code!=G_KEY_FILE_ERROR_KEY_NOT_FOUND)
             && (err->code!=G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
        throw (runtime_error((group+"::"+key+" option value is invalid.").c_str()));

    return rval;
}

/**
 * @brief Get the value of an integer key. If key does not exist, its default value is returned
 * @param[in] a group name
 * @param[in] a key
 * @param[in] default value
 * @return retrieved key value or default value if key does not exist
 * @throw runtime_error if the key is not integer
 */
int CGKeyFileBase::GetIntegerKey(const std::string &group, const std::string &key, int rval)
{
    GError *err = NULL;
    int val = g_key_file_get_integer(key_file, group.c_str(), key.c_str(), &err);
    if (!err)
        rval = val;
    else if (err && (err->code!=G_KEY_FILE_ERROR_KEY_NOT_FOUND)
            && (err->code!=G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
        throw (runtime_error((group+"::"+key+" option value is invalid.").c_str()));

    return rval;
}

/**
 * @brief Get the value of a string key. If key does not exist, its default value is returned
 * @param[in] a group name
 * @param[in] a key
 * @param[in] default value
 * @return retrieved key value or default value if key does not exist
 * @throw runtime_error if the key is not string
 */
std::string CGKeyFileBase::GetStringKey(const std::string &group, const std::string &key,
                         std::string rval)
{
    GError *err = NULL;
    char* val = g_key_file_get_string(key_file, group.c_str(), key.c_str(), &err);
    try
    {
        if (!err)
            rval = val;
        else if ((err->code!=G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                 && (err->code!=G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
            throw (runtime_error((group+"::"+key+" option value is invalid.").c_str()));
    }
    catch(...)
    {
        g_free(val);
        throw;  // rethrow the exception
    }

    g_free(val);
    return rval;
}

/**
 * @brief Get the value of a string list key. If key does not exist, a default value is returned
 * @param[in] a group name
 * @param[in] a key
 * @param[in] default value
 * @return retrieved key value or default value if key does not exist
 * @throw runtime_error if the key is not a string list
 */
std::vector<std::string> CGKeyFileBase::GetStringListKey(const std::string &group, const std::string &key,
                                          const std::vector<std::string> &defval)
{
    std::vector<string> rval;
    size_t len;
    GError *err = NULL;
    char **strlist = g_key_file_get_string_list(key_file, group.c_str(), key.c_str(), &len, &err);
    try
    {
        if (!err)
        {
            rval.reserve(len);
            for (size_t i=0;i<len;i++) rval.emplace_back(strlist[i]);
        }
        else if ((err->code==G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                 || (err->code==G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
        {
            rval = defval;
        }
        else
        {
            throw (runtime_error((group+"::"+key+" option value is invalid.").c_str()));
        }
    }
    catch(...)
    {
        g_strfreev (strlist);
        throw;  // rethrow the exception
    }

    g_strfreev (strlist);
    return rval; // should never get here
}
