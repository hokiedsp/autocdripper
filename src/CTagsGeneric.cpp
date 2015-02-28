// $Id$
/**
 * @file CTagsGeneric.cpp
 * Implementation of CTagsGeneric class, declared in CTagsGeneric.h
 *
 * @brief Can use "brief" tag to explicitly generate comments for file documentation.
 */
// $Log$
 
#include "CTagsGeneric.h"

#include <cstring>

using std::string;
using std::deque;

/**
   Constructor
 */
CTagsGeneric::CTagsGeneric() : itvalid(false) {}

/**
   Deconstructor
 */
CTagsGeneric::~CTagsGeneric()
{
	// delete all the tags (no need to NULL them)
	for (it = table.begin(); it!=table.end(); it++)
	{
		if (*it) delete *it;
	}
}

/**
   Appends a new tag to the object.

   @param[in]     new tag's key word
   @param[in]    	new tag's value
 */
void CTagsGeneric::AppendTag(const std::string &key, const std::string &val)
{
	// look for a duplicate key
	deque<STagGeneric*>::iterator i;
	bool notfound = SearchTag_(key, i);

	// insert new element at the end or update the existing tag if keyword found
	if (notfound) table.push_back(NewTag_(key,val));
	else (*i)->Update(val);
}

void CTagsGeneric::AppendTag(const std::string &key, const char *data, const size_t sz)
{
	// look for a duplicate key
	deque<STagGeneric*>::iterator i;
	bool notfound = SearchTag_(key, i);

	// insert new element at the end or update the existing tag if keyword found
	if (notfound) table.push_back(NewTag_(key,data,sz));
	else (*i)->Update(data,sz);
}

bool CTagsGeneric::SearchTag_(const std::string &key, deque<STagGeneric*>::iterator &it)
{
	bool notfound = true;

	// look for a duplicate key
	for (it = table.begin(); it!=table.end() && notfound; it++)
		notfound = (0!=(*it)->key.compare(key));
	
	return notfound;
}


STagGeneric* CTagsGeneric::NewTag_(const std::string &key, const char *data, const size_t sz)
{
	return new STagGeneric(key,data,sz);
}

STagGeneric* CTagsGeneric::NewTag_(const std::string &key, const std::string &val)
{
	return new STagGeneric(key,val);
}
	
const STagGeneric* CTagsGeneric::ReadFirstTag()
{
	if (table.empty())
	{
		// if no tag defined, return empty
		return NULL;
	}
	else
	{
		// set the iterator to the top of the tag table
		it = table.begin();
		itvalid = true;
		return *it;
	}
}

const STagGeneric* CTagsGeneric::ReadNextTag()	// returns NULL if no more
{
	if (itvalid)
	{
		if ((++it)!=table.end())
		{
			return *it;
		}
		else
		{
			itvalid = false;
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}

