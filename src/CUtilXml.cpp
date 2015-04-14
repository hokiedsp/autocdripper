#include "CUtilXml.h"

#include <stdexcept>
#include <iostream>

using std::string;
using std::mutex;
using std::runtime_error;

// initialize static member variables
int CUtilXml::Nobjs = 0;
std::atomic_bool CUtilXml::AutoCleanUp(true);
std::mutex CUtilXml::globalmutex;

/** Constructor.
 *
 *  @param[in] Client program name. If omitted or empty, uses "autorip"
 *  @param[in] Client program version. If omitted or empty, uses "alpha"
 */
CUtilXml::CUtilXml()
{
    // initialize curl object
    globalmutex.lock();
    if (!Nobjs) xmlInitParser(); // only iniitialize XML library for the first instantiation
    Nobjs++;
    globalmutex.unlock();
}

/** Destructor
 */
CUtilXml::~CUtilXml()
{
    // decrement the number of instances and call global cleanup if this is the last object
    globalmutex.lock();
    Nobjs--;
    if (!Nobjs && AutoCleanUp) xmlCleanupParser();
    globalmutex.unlock();
}

bool CUtilXml::FindElement(const xmlNode *parent, const std::string &key, xmlNode *&node)
{
    for (node = parent->children;
         node && node->type==XML_ELEMENT_NODE && (key.compare((char*)node->name)!=0);
         node = node->next);
    return node!=NULL;
}

bool CUtilXml::FindNextElement(const xmlNode *curr, const std::string &key, xmlNode *&node)
{
    for (node = curr->next;
         node && node->type==XML_ELEMENT_NODE && (key.compare((char*)node->name)!=0);
         node = node->next);
    return node!=NULL;
}

bool CUtilXml::FindElementAttribute(const xmlNode *node, const std::string &name, std::string &value)
{
    bool rval;
    xmlAttrPtr attr;

    // look for the attribute
    for (attr = node->properties; attr && (name.compare((char*)attr->name)!=0); attr = attr->next);

    // attribute found, grab the value (expects only 1 child (a text node)
    if ((rval = attr && attr->children)) value = (char*)(attr->children->content);

    return rval;
}

int CUtilXml::CompareElementAttribute(const xmlNode *node, const std::string &name, const std::string &value)
{
    int rval = -1;
    xmlAttrPtr attr;

    // look for the attribute
    for (attr = node->properties; attr && (name.compare((char*)attr->name)!=0); attr = attr->next);

    // attribute found, grab the value (expects only 1 child (a text node)
    if (attr && attr->children) value.compare((char*)(attr->children->content));

    return rval;
}

