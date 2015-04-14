#pragma once

#include <atomic>
#include <mutex>

#include <libxml/parser.h>

/** A base class of all libxml2 wrapper objects to execute libxml2 library's global initialization/cleanup functions
 */
class CUtilXml
{
public:
    /** Constructor.
     *
     *  @param[in] Client program name. If omitted or empty, uses "autorip"
     *  @param[in] Client program version. If omitted or empty, uses "alpha"
     */
    CUtilXml();

    /** Destructor
     */
    virtual ~CUtilXml();

    /** Get current value of static AutoCleanUpMode parameter
     *
     *  @return true if curl_global_cleanup is to be called automatically.
     */
    static bool GetAutoCleanUpMode(const bool mode) { return AutoCleanUp; }

    /** Set static AutoCleanUpMode parameter
     *
     *  @param[in] true if curl_global_cleanup is to be called automatically.
     */
    static void SetAutoCleanUpMode(const bool mode) { AutoCleanUp = mode; }


    static bool FindElement(const xmlNode *parent, const std::string &key, xmlNode *&node);
    static bool FindNextElement(const xmlNode *curr, const std::string &key, xmlNode *&node);
    static bool FindElementAttribute(const xmlNode *node, const std::string &name, std::string &value);
    static int CompareElementAttribute(const xmlNode *node, const std::string &name, const std::string &value);

protected:

private:
    static std::mutex globalmutex; /// mutex to make curl_global_init and curl_global_cleanup thread safe
    static int Nobjs; /// number of instantiated CUtilXml objects
    static std::atomic_bool AutoCleanUp;    /// if true (default)

};
