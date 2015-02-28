#pragma once

#include <vector>
#include <deque> 
#include <string>

/**
 * struct for a basic tag.
 */
struct STagGeneric
{
	std::string key;	/// tag keyword
	std::vector<char> val; // tag value

	STagGeneric(const std::string &k, const char* v, const size_t sz):
		key(k), val(v,v+sz) {}
	STagGeneric(const std::string &k, const std::string &v):
		key(k), val(v.begin(),v.end()) {}
	STagGeneric(const std::string &k, const std::vector<char> &v):
		key(k), val(v) {}
	virtual ~STagGeneric() {}

	virtual void Update(const char* v, const size_t sz)
	{
		val.assign(v,v+sz);
	}
	virtual void Update(const std::string &v)
	{
		val.assign(v.begin(),v.end());
	}
	virtual void Update(const std::vector<char> &v)
	{
		val.assign(v.begin(),v.end());
	}
};

// Base tags class
class CTagsGeneric
{
public:
	CTagsGeneric();
	virtual ~CTagsGeneric();

	/**
   Creates and appends a new tag, and store it within the object. The type of the created
   tag object depends on the derived class type. By default, it stores STagGeneric object.
   
   If it fails, the function throws a std::runtime_error.

   @param[in]     new tag's keyword
   @param[in]    	new tag's value
	*/
	virtual void AppendTag(const std::string &key, const std::string &val);

	/**
   Creates and appends a new tag, and store it within the object. The type of the created
   tag object depends on the derived class type. By default, it stores STagGeneric object.
   
   If it fails, the function throws a std::runtime_error.

   @param[in]     new tag's keyword
   @param[in]    	new tag's value
   @param[in]     length of the value array
	*/
	virtual void AppendTag(const std::string &key, const char *data, const size_t sz);

	/**
   Returns the first tag, and initializes its internal iterator so that a subsequent
   ReadNextTag() returns the next tag.

   @return   A const STagGeneric pointer to the first tag object. The returned pointer
             shall be casted to the appropriate Tag datatype. CTagsGeneric object by
             default returns a pointer to its STagGeneric object.
	 */
	virtual const STagGeneric* ReadFirstTag(); // configures the iterator and returns the first tag

	/**
   Returns the next tag, and increments its internal iterator. If the last tag has already 
   been returned, it returns NULL.

   @return   A const STagGeneric pointer to the first tag object. The returned pointer
             shall be casted to the appropriate Tag datatype. CTagsGeneric object by
             default returns a pointer to its STagGeneric object. If tags have been exausted
             a NULL pointer shall be returned.
	 */
	virtual const STagGeneric* ReadNextTag();	// returns empty if no more
	
protected:
	std::deque<STagGeneric*> table;
	std::deque<STagGeneric*>::iterator it;	// iterator for ReadFirstTag & ReadNextTag
	bool itvalid;	// true if iterator is valid
	
	/**
   Searches through existing tags in deque table for the matched keyword.

   @param[in]     keyword
   @param[out]    iterator to deque table
   @return   		true if no matching keyword is found or false if found
	*/
	virtual bool SearchTag_(const std::string &key, std::deque<STagGeneric*>::iterator &it);

	/**
   Returns the next tag, and increments its internal iterator. If the last tag has already 
   been returned, it returns NULL.

   @return   A const STagGeneric pointer to the first tag object. The returned pointer
             shall be casted to the appropriate Tag datatype. CTagsGeneric object by
             default returns a pointer to its STagGeneric object. If tags have been exausted
             a NULL pointer shall be returned.
	 */
	virtual STagGeneric* NewTag_(const std::string &key, const char *data, const size_t sz);
	virtual STagGeneric* NewTag_(const std::string &key, const std::string &val);
};

