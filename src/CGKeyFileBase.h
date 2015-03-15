#pragma once

#include <string>
#include <vector>

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

private:
    std::string file_name;

    /**
     * @brief Word-wrap description text and insert it to the comment text
     * @param[out] key file comment text, description will be appended
     * @param[in] description text without word-warpping.
     */
    void InsertDescription_(std::string &comment, std::string text);
};
