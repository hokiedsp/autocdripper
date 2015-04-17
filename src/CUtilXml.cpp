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
