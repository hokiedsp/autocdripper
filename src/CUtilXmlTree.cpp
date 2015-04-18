#include "CUtilXmlTree.h"

#include <iostream>

#include <utility>
#include <stdexcept>
#include <cstring>

using std::runtime_error;

/** Constructor.
 */
CUtilXmlTree::CUtilXmlTree(const std::string &rawdata) : root(nullptr), data(nullptr)
{
    if (rawdata.size())
    {
        /*
    * The document being in memory, it have no base per RFC 2396,
    * and the "noname.xml" argument will serve as its base.
    */
        data = xmlReadMemory(rawdata.c_str(), rawdata.size(), "", nullptr, 0);
        if (data) root = xmlDocGetRootElement(data);
        else throw(std::runtime_error("Failed to parse document\n"));
    }
}

/**
 * @brief copy constructor
 * @param source object
 */
CUtilXmlTree::CUtilXmlTree(const CUtilXmlTree &src) : root(nullptr), data(nullptr)
{
    data = xmlCopyDoc(src.data,1);
    if (!data) throw(std::runtime_error("Failed to copy XML document."));
}

/** Destructor
 */
CUtilXmlTree::~CUtilXmlTree()
{
    if (data) xmlFreeDoc(data);
}

/**
 * @brief Exchanges the content with another CDbMusicBrainzElem object
 * @param Another CDbMusicBrainzElem object
 */
void CUtilXmlTree::Swap(CUtilXmlTree &other)
{
    std::swap(data,other.data);
    std::swap(root,other.root);
}

void CUtilXmlTree::LoadData(const std::string &rawdata)
{
    if (rawdata.size())
    {
        // clear existing data
        ClearData();

        /*
        * The document being in memory, it have no base per RFC 2396,
        * and the "noname.xml" argument will serve as its base.
        */
        data = xmlReadMemory(rawdata.c_str(), rawdata.size(), "", nullptr, 0);

        if (data) root = xmlDocGetRootElement(data);
        else throw(std::runtime_error("Failed to parse document\n"));
    }
}

/**
 * @brief Clear existing JSON data
 */
void CUtilXmlTree::ClearData()
{
    if (data)
    {
        xmlFreeDoc(data);
        data = nullptr;
        root = nullptr;
    }
}

bool CUtilXmlTree::FindElement(const xmlNode *parent, const std::string &key, const xmlNode *&node)
{
    for (node = parent->children;
         node && node->type==XML_ELEMENT_NODE && (key.compare((char*)node->name)!=0);
         node = node->next);
    return node!=nullptr;
}

bool CUtilXmlTree::FindNextElement(const xmlNode *curr, const std::string &key, const xmlNode *&node)
{
    for (node = curr->next;
         node && node->type==XML_ELEMENT_NODE && (key.compare((char*)node->name)!=0);
         node = node->next);
    return node!=nullptr;
}

bool CUtilXmlTree::FindElementAttribute(const xmlNode *node, const std::string &name, std::string &value)
{
    bool rval;
    xmlAttrPtr attr;

    // look for the attribute
    for (attr = node->properties; attr && (name.compare((char*)attr->name)!=0); attr = attr->next);

    // attribute found, grab the value (expects only 1 child (a text node)
    if ((rval = attr && attr->children)) value = (char*)(attr->children->content);

    return rval;
}

int CUtilXmlTree::CompareElementAttribute(const xmlNode *node, const std::string &name, const std::string &value)
{
    int rval = -1;
    xmlAttrPtr attr;

    // look for the attribute
    for (attr = node->properties; attr && (name.compare((char*)attr->name)!=0); attr = attr->next);

    // attribute found, grab the value (expects only 1 child (a text node)
    if (attr && attr->children) rval = value.compare((char*)(attr->children->content));

    return rval;
}

bool CUtilXmlTree::FindArray(const xmlNode *parent, const std::string &name, const xmlNode *&node, int *count)
{
    // find a node with requested name
    bool rval = FindElement(parent, name, node);

    if (rval)   // found
    {
        // if count requested, return the number of children
        if (count) *count = xmlChildElementCount(const_cast<xmlNode*>(node));

        // return its first child
        node =  node->children;
    }

    return rval;
}

bool CUtilXmlTree::FindString(const xmlNode *parent, const std::string &key, std::string &val)
{
    const xmlNode *node;
    bool rval = FindElement(parent, key, node);

    if (rval) // a node with key as its name found
    {
        node = node->children;
        rval = node && (node->type==XML_TEXT_NODE);
        if (rval) val = (char*)node->content;
    }

    return rval;
}

bool CUtilXmlTree::FindInt(const xmlNode *node, const std::string &key, int &val)
{
    std::string str;
    bool rval = FindString(node, key, str);
    if (rval)
    {
        try
        {
            size_t endpos;
            val = std::stoi(str, &endpos);
            rval = (endpos==str.size()); // something is following the integer... fail
        }
        catch(...) // conversion failed -> not an integer
        {
            rval = false;
        }
    }
    return rval;
}

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
int CUtilXmlTree::CompareString(const xmlNode *parent, const std::string &key, const std::string &str)
{
    const xmlNode *node;
    int rval = -1; // if remained, key doesn't exist or its value is not a string

    if (FindElement(parent, key, node))
    {
        node = node->children;
        if (node && (node->type==XML_TEXT_NODE)) rval = str.compare((char*)node->content);
    }
    return rval;
}

void CUtilXmlTree::PrintXmlTree(const xmlNode *obj, const int depth, std::ostream &os)
{
    PrintXmlTree_(os, depth, obj, "");
}

void CUtilXmlTree::PrintXmlTree_(std::ostream &os, int depth, const xmlNode *obj, std::string indent)
{
    if (obj->type==XML_ELEMENT_NODE)
    {
        // string node
        if (obj->children && (obj->children->type==XML_TEXT_NODE))
        {
            os << indent << obj->name << "(s)";
            if (obj->properties)
            {
                _xmlAttr* attr = obj->properties;

                os << "[" << attr->name;
                if (attr->children) os << "=" << attr->children->content;
                attr = attr->next;
                while (attr)
                {
                    os << ", " << attr->name;
                    if (attr->children) os << "=""" << attr->children->content << """";
                    attr = attr->next;
                }
                os << "]";
            }
            os << " " << obj->children->content << std::endl;
        }
        else
        {
            os << indent << obj->name;

            // list node must have "list" in its name
            if (strstr((char*)obj->name,"list")) os << "(l)";
            else os << "(o)";

            if (obj->properties)
            {
                _xmlAttr* attr = obj->properties;

                os << "[" << attr->name;
                if (attr->children) os << "=" << attr->children->content;
                attr = attr->next;
                while (attr)
                {
                    os << ", " << attr->name;
                    if (attr->children) os << "=""" << attr->children->content << """";
                    attr = attr->next;
                }
                os << "]";
            }
            os << std::endl;

            // next level down
            if (depth!=0)
            {
                depth--;
                indent += " ";
                for (obj = obj->children; obj; obj = obj->next)
                    PrintXmlTree_(os, depth, obj, indent);
            }
        }
    }
    else    // just in case unknown node is present
    {
        xmlBuf *buf=nullptr;
        xmlBufNodeDump(buf, obj->doc, const_cast<xmlNode*>(obj), indent.size(), 0);
        if (buf) os << (char*)buf << std::endl;
    }
}
