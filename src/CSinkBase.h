#pragma once

#include <cstdint>
#include <string>

#include "ISink.h"

class CSinkBase : ISink
{
public:
	CSinkBase(const std::string &path);
	virtual ~CSinkBase();

protected:
	virtual void SeekFile_(const long int offset, const int origin );
	virtual size_t ReadFile_(void* buf, const size_t N);
	virtual size_t WriteFile_(const void *buf, const size_t N);
	virtual bool EOF_(); // returns true if end-of-file
	virtual size_t GetNumberOfBytesWritten_();
private:
	FILE* file;
	size_t nbytes_total;
};

