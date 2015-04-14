#include "CDbMusicBrainzElemBase.h"

#include <iostream>

#include <utility>
#include <stdexcept>
#include <cstring>

using std::runtime_error;

/** Constructor.
 */
CDbMusicBrainzElemBase::CDbMusicBrainzElemBase(const std::string &rawdata) : root(NULL), data(NULL)
{
    if (rawdata.size())
    {
        /*
    * The document being in memory, it have no base per RFC 2396,
    * and the "noname.xml" argument will serve as its base.
    */
        data = xmlReadMemory(rawdata.c_str(), rawdata.size(), "", NULL, 0);
        if (data) root = xmlDocGetRootElement(data);
        else throw(std::runtime_error("Failed to parse document\n"));
    }
}

/**
 * @brief copy constructor
 * @param source object
 */
CDbMusicBrainzElemBase::CDbMusicBrainzElemBase(const CDbMusicBrainzElemBase &src) : root(NULL), data(NULL)
{
    data = xmlCopyDoc(src.data,1);
    if (!data) throw(std::runtime_error("Failed to copy XML document."));
}

/** Destructor
 */
CDbMusicBrainzElemBase::~CDbMusicBrainzElemBase()
{
    if (data) xmlFreeDoc(data);
}

/**
 * @brief Exchanges the content with another CDbMusicBrainzElem object
 * @param Another CDbMusicBrainzElem object
 */
void CDbMusicBrainzElemBase::Swap(CDbMusicBrainzElemBase &other)
{
    std::swap(data,other.data);
    std::swap(root,other.root);
}

void CDbMusicBrainzElemBase::LoadData(const std::string &rawdata)
{
    if (rawdata.size())
    {
        // clear existing data
        ClearData();

        /*
        * The document being in memory, it have no base per RFC 2396,
        * and the "noname.xml" argument will serve as its base.
        */
        data = xmlReadMemory(rawdata.c_str(), rawdata.size(), "", NULL, 0);

        if (data) root = xmlDocGetRootElement(data);
        else throw(std::runtime_error("Failed to parse document\n"));
    }
}

/**
 * @brief Clear existing JSON data
 */
void CDbMusicBrainzElemBase::ClearData()
{
    if (data)
    {
        xmlFreeDoc(data);
        data = NULL;
        root = NULL;
    }
}

bool CDbMusicBrainzElemBase::FindObject(const xmlNode *parent, const std::string &name, xmlNode *&node, std::string *id)
{
    // find a node with requested name
    bool rval = FindElement(parent, name, node);

    // if found and its ID URI is requested, try retrieving it (returns "" if does not exist)
    if (rval && id) FindElementAttribute(node, "id", *id);

    return rval;
}

bool CDbMusicBrainzElemBase::FindArray(const xmlNode *parent, const std::string &name, xmlNode *&node, int *count, int *offset)
{
    // find a node with requested name
    bool rval = FindElement(parent, name, node);

    if (rval)   // found
    {
        std::string attr_val;

        if (count)
        {
            FindElementAttribute(node, "count", attr_val);
            try
            {
                *count = std::stoi(attr_val);
            }
            catch(...)  // if attribute does not exist or not integer, count the # of children
            {
                *count = xmlChildElementCount(node);
            }
        }

        if (offset)
        {
            FindElementAttribute(node, "offset", attr_val);
            try
            {
                *offset = std::stoi(attr_val);
            }
            catch(...)  // if attribute does not exist or not integer, count the # of children
            {
                *offset = -1; // unknown
            }
        }

        // return its first child
        node =  node->children;
    }

    return rval;
}

bool CDbMusicBrainzElemBase::FindString(const xmlNode *parent, const std::string &key, std::string &val)
{
    xmlNodePtr node;
    bool rval = FindElement(parent, key, node);

    if (rval) // a node with key as its name found
    {
        node = node->children;
        rval = node && (node->type==XML_TEXT_NODE);
        if (rval) val = (char*)node->content;
    }

    return rval;
}

bool CDbMusicBrainzElemBase::FindInt(const xmlNode *node, const std::string &key, int &val)
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
int CDbMusicBrainzElemBase::CompareString(const xmlNode *parent, const std::string &key, const std::string &str)
{
    xmlNodePtr node;
    int rval = -1; // if remained, key doesn't exist or its value is not a string

    if (FindElement(parent, key, node))
    {
        node = node->children;
        if (node && (node->type==XML_TEXT_NODE)) rval = str.compare((char*)node->content);
    }
    return rval;
}

void CDbMusicBrainzElemBase::PrintXML(const xmlNode *obj, const int depth, std::ostream &os)
{
    PrintXML_(os, depth, obj, "");
}

void CDbMusicBrainzElemBase::PrintXML_(std::ostream &os, int depth, const xmlNode *obj, std::string indent)
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
                    PrintXML_(os, depth, obj, indent);
            }
        }
    }
    else    // just in case unknown node is present
    {
        xmlBufPtr buf=NULL;
        xmlBufNodeDump(buf, obj->doc, const_cast<xmlNode*>(obj), indent.size(), 0);
        if (buf) os << (char*)buf << std::endl;
    }
}
