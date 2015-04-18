#pragma once

#include <string>
#include <iostream>

#include "libxml/tree.h"
#include "CUtilXml.h"

/** Abstract base Database class with CD info stored in JSON format
 */
class CUtilXmlTree : public CUtilXml
{
public:
    const xmlNode *root;

    /** Constructor.
     */
    CUtilXmlTree(const std::string &data="");
    CUtilXmlTree(const CUtilXmlTree &src);

    /** Destructor
     */
    virtual ~CUtilXmlTree();

    /**
     * @brief Exchanges the content with another CDbMusicBrainzElem object
     * @param Another CDbMusicBrainzElem object
     */
    virtual void Swap(CUtilXmlTree &other);

    /**
     * @brief Load New Data. Clears existing data first. If empty, no action.
     * @param valid XML string following musicbrainz_mmd-2.0.rng
     * @throw runtime_error if invalid XML string is passed in.
     */
    virtual void LoadData(const std::string &data);

    /**
     * @brief Clear existing XML data
     */
    virtual void ClearData();

    /** Debug function. Prints full-struct of XML tree for a release
     *
     * @param[in] Depth to traverse the XML object tree (negative to go all the way)
     * @param[in] Record index to print'
     * @param[in] Output stream to print the object (default: cout)
     */
    virtual void PrintXMLTree(const int depth=-1, std::ostream &os=std::cout) const { PrintXmlTree(root, depth, os); }

    // functions to get value of a requested key off the root
    bool FindElement(const std::string &key, const xmlNode *&node) const { return FindElement(root, key, node); }
    bool FindArray(const std::string &key, const xmlNode *&firstchild, int *count=NULL) const { return FindArray(root, key, firstchild, count); }
    bool FindString(const std::string &key, std::string &val) const { return FindString(root, key, val); }
    bool FindInt(const std::string &key, int &val) const { return FindInt(root, key, val); }

    /**
     * @brief CompareString_ compare string root key value to the given string
     * @param[in] JSON element key word
     * @param[in] String object to compare to (the comparing string)
     * @return     A signed integral indicating the relation between the strings: 0 if equal,
     *             <0 if key is invalid or not a string key or key string is longer than
     *             the comparing string, or >0 if first key string is shorter than the
     *             comparing string
     */
    int CompareString(const std::string &key, const std::string &str) const { return CompareString(root, key, str); }

    static void PrintXmlTree(const xmlNode *node, const int depth=-1, std::ostream &os=std::cout);

    // generic static functions to get value of a requested key

    static bool FindElement(const xmlNode *parent, const std::string &key, const xmlNode *&node);
    static bool FindNextElement(const xmlNode *curr, const std::string &key, const xmlNode *&node);
    static bool FindArray(const xmlNode *parent, const std::string &key, const xmlNode *&firstchild, int *count=NULL);
    static bool FindString(const xmlNode *parent, const std::string &key, std::string &val);
    static bool FindInt(const xmlNode *parent, const std::string &key, int &val);

    static bool FindElementAttribute(const xmlNode *node, const std::string &name, std::string &value);
    static int CompareElementAttribute(const xmlNode *node, const std::string &name, const std::string &value);

    /**
     * @brief CompareString_ compare string key value to the given string
     * @param[in] Pointer to the parent XML tree node
     * @param[in] name of the child XML element node under comparison
     * @param[in] String object to compare to (the compared string)
     * @return     A signed integral indicating the relation between the strings: 0 if equal,
     *             <0 if key is invalid or not a string key or key string is longer than
     *             the compared string, or >0 if first key string is shorter than the
     *             compared string
     */
    static int CompareString(const xmlNode *obj, const std::string &key, const std::string &str);
private:
    xmlDocPtr data;

    static void PrintXmlTree_(std::ostream &os, int depth, const xmlNode *obj, std::string indent);
};
