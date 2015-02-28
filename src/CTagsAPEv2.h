#pragma once
// http://audacity.googlecode.com/svn/audacity-src/trunk/lib-src/taglib/taglib/ape/ape-tag-format.txt

#include "CTagsGeneric.h"

#define APEv2_MAXSIZE 8192

#define APEv2_KEY_MINSIZE 2
#define APEv2_KEY_MAXSIZE 255

#define APEv2_ENC_UTF8   0
#define APEv2_ENC_BINARY 1
#define APEv2_ENC_EXTREF 2

struct STagAPEv2 : STagGeneric
{
	int enc;

	STagAPEv2(const std::string &k, const char* v, const size_t sz, const int e=APEv2_ENC_UTF8):
		STagGeneric(k,v,sz), enc(e) {}
	STagAPEv2(const std::string &k, const std::string &v, const int e=APEv2_ENC_UTF8):
		STagGeneric(k,v), enc(e) {}
	STagAPEv2(const std::string &k, const std::vector<char> &v):
		STagGeneric(k,v), enc(APEv2_ENC_BINARY) {}
	virtual ~STagAPEv2() {}
};

class CTagsAPEv2 : public CTagsGeneric
{
public:
	virtual ~CTagsAPEv2(){}

	virtual void AppendBinaryTag(const std::string &key, const std::vector<char> &val);
	virtual void AppendBinaryTag(const std::string &key, const char *data, const size_t sz);
	
	virtual void AppendExtRefTag(const std::string &key, const std::string &val);
	virtual void AppendExtRefTag(const std::string &key, const char *data, const size_t sz);

	//virtual const STagGeneric* ReadFirstTag();
	//virtual const STagGeneric* ReadNextTag();	// returns empty if no more
protected:

	virtual STagGeneric* NewTag_(const std::string &key, const char *data, const size_t sz,
										  const int enc=APEv2_ENC_UTF8);
	virtual STagGeneric* NewTag_(const std::string &key, const std::string &val,
										  const int enc=APEv2_ENC_UTF8);
	virtual STagGeneric* NewTag_(const std::string &key, const std::vector<char> &val);

};


