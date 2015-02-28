#include "CTagsAPEv2.h"

#include <cstring>
#include <stdexcept>

using std::deque;

void CTagsAPEv2::AppendBinaryTag(const std::string &key, const std::vector<char> &val)
{
	// look for a duplicate key
	deque<STagGeneric*>::iterator i;
	bool notfound = SearchTag_(key, i);

	// insert new element at the end or update the existing tag if keyword found
	if (notfound) table.push_back(NewTag_(key,val));
	else (*i)->Update(val);
}

void CTagsAPEv2::AppendBinaryTag(const std::string &key, const char *data, const size_t sz)
{
	// look for a duplicate key
	deque<STagGeneric*>::iterator i;
	bool notfound = SearchTag_(key, i);

	// insert new element at the end or update the existing tag if keyword found
	if (notfound) table.push_back(NewTag_(key,data,sz,APEv2_ENC_BINARY));
	else (*i)->Update(data,sz);
}

void CTagsAPEv2::AppendExtRefTag(const std::string &key, const std::string &val)
{
	// look for a duplicate key
	deque<STagGeneric*>::iterator i;
	bool notfound = SearchTag_(key, i);

	// insert new element at the end or update the existing tag if keyword found
	if (notfound) table.push_back(NewTag_(key,val,APEv2_ENC_EXTREF));
	else (*i)->Update(val);
}

void CTagsAPEv2::AppendExtRefTag(const std::string &key, const char *data, const size_t sz)
{
	// look for a duplicate key
	deque<STagGeneric*>::iterator i;
	bool notfound = SearchTag_(key, i);

	// insert new element at the end or update the existing tag if keyword found
	if (notfound) table.push_back(NewTag_(key,data,sz,APEv2_ENC_EXTREF));
	else (*i)->Update(data,sz);
}

STagGeneric* CTagsAPEv2::NewTag_(const std::string &key, const char *data, const size_t sz,
										  const int enc)
{
	return (STagGeneric*) new STagAPEv2(key,data,sz,enc);
}

STagGeneric* CTagsAPEv2::NewTag_(const std::string &key, const std::string &val,
										  const int enc)
{
	return (STagGeneric*) new STagAPEv2(key,val,enc);
}
	
STagGeneric* CTagsAPEv2::NewTag_(const std::string &key, const std::vector<char> &val)
{
	return (STagGeneric*) new STagAPEv2(key,val);
}

