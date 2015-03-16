#pragma once

#include <string>
#include <vector>
#include <stdexcept>

#include <glib.h>

class CGKeyFileBase
{
public:
    CGKeyFileBase(const std::string &filename);
    virtual ~CGKeyFileBase();

    virtual void Save();

protected:
    GKeyFile *key_file;

    /**
     * @brief Create a new bool key with default value and key description
     * @param[in] a group name
     * @param[in[ a key
     * @param[in] the default value of the the key
     * @param[in] description of the key
     */
    void CreateBooleanKey(const std::string &group, const std::string &key,
                          const bool defval, const std::string &description);

    /**
     * @brief Create a new integer key with default value and key description
     * @param[in] a group name
     * @param[in[ a key
     * @param[in] the default value of the the key
     * @param[in] description of the key
     */
    void CreateIntegerKey(const std::string &group, const std::string &key,
                          const int defval, const std::string &description);

    /**
     * @brief Create a new string key with default value and key description
     * @param[in] a group name
     * @param[in[ a key
     * @param[in] possible key values
     * @param[in] the default value of the the key
     * @param[in] description of the key
     */
    void CreateStringKey(const std::string &group, const std::string &key,
                         const std::vector<std::string> optvals,
                         const std::string defval,
                         const std::string &description);

    /**
     * @brief Create a new string key with default value and key description
     * @param[in] a group name
     * @param[in[ a key
     * @param[in] the default value of the the key
     * @param[in] description of the key
     */
    void CreateStringKey(const std::string &group, const std::string &key,
                         const std::string defval,
                         const std::string &description);

    /**
     * @brief Create a new string list key with default value and key description
     * @param[in] a group name
     * @param[in[ a key
     * @param[in] possible key values
     * @param[in] the default value of the the key
     * @param[in] description of the key
     */
    void CreateStringListKey(const std::string &group, const std::string &key,
                             const std::vector<std::string> optvals,
                             const std::vector<std::string> defval,
                             const std::string &description);

    ///////////////////////////////////////////////

    /**
     * @brief Get the value of a boolean key. If key does not exist, its default value is returned
     * @param[in] a group name
     * @param[in] a key
     * @param[in] default value
     * @return retrieved key value or default value if key does not exist
     * @throw runtime_error if the key is not boolean
     */
    bool GetBooleanKey(const std::string &group, const std::string &key, bool defval);

    /**
     * @brief Get the value of an integer key. If key does not exist, its default value is returned
     * @param[in] a group name
     * @param[in] a key
     * @param[in] default value
     * @return retrieved key value or default value if key does not exist
     * @throw runtime_error if the key is not integer
     */
    int GetIntegerKey(const std::string &group, const std::string &key, int defval);

    /**
     * @brief Get the value of a string key. If key does not exist, its default value is returned
     * @param[in] a group name
     * @param[in] a key
     * @param[in] default value
     * @return retrieved key value or default value if key does not exist
     * @throw runtime_error if the key is not string
     */
    std::string GetStringKey(const std::string &group, const std::string &key,
                             std::string defval);

    /**
     * @brief Get the value of a string key and convert it to another type.
     *        If key does not exist, its default value is returned
     * @param[in] a group name
     * @param[in] a key
     * @param[in] default value
     * @param[in] a pointer to conversion function
     * @return retrieved key value or default value if key does not exist
     * @throw runtime_error if the key is not string
     * @throw an exception if conversaion failed
     */
    template <class T>
    T GetStringKey(const std::string &group, const std::string &key, T rval,
                   T (*convert)(const std::string &val))
    {
        GError *err = NULL;
        char* val = g_key_file_get_string(key_file, group.c_str(), key.c_str(), &err);
        try
        {
            if (!err)
                rval = convert(val);
            else if ((err->code!=G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                     && (err->code!=G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
                throw (std::runtime_error((group+"::"+key+" option value is invalid.").c_str()));
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
     * @throw runtime_error if the key is not string list
     */
    std::vector<std::string> GetStringListKey(const std::string &group, const std::string &key,
                                              const std::vector<std::string> &defval);

    /**
     * @brief Get the value of a string list key and convert it to a vector of another type.
     *        If key does not exist, a default value is returned
     * @param[in] a group name
     * @param[in] a key
     * @param[in] default value
     * @param[in] a pointer to conversion function
     * @return retrieved key value or default value if key does not exist
     * @throw runtime_error if the key is not a string list
     * @throw an exception if conversaion failed
     */
    template <class T>
    std::vector<T> GetStringListKey(const std::string &group, const std::string &key,
                                    const std::vector<T> &defval,
                                    T (*convert)(const std::string &val))
    {
        std::vector<T> rval;
        size_t len;
        GError *err = NULL;
        char **strlist = g_key_file_get_string_list(key_file, group.c_str(), key.c_str(), &len, &err);
        try
        {
            if (!err)
            {
                rval.reserve(len);
                for (int i=0;i<len;i++) rval.push_back(convert(strlist[i]));
            }
            else if ((err->code==G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                     || (err->code==G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
            {
                rval = defval;
            }
            else
            {
                throw (std::runtime_error((group+"::"+key+" option value is invalid.").c_str()));
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

private:
    std::string file_name;

    /**
     * @brief Word-wrap description text and insert it to the comment text
     * @param[out] key file comment text, description will be appended
     * @param[in] description text without word-warpping.
     */
    void InsertDescription_(std::string &comment, std::string text);
};
