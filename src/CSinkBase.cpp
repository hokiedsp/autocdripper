#include "CSinkBase.h"

#include <stdexcept>
#include <cstdio>

using std::string;
using std::runtime_error;

CSinkBase::CSinkBase(const std::string &path) : nbytes_total(0)
{
	file = fopen(path.c_str(), "w+b");
	if (!file) throw(runtime_error("Could not open the output file."));
}

CSinkBase::~CSinkBase()
{
	if (file) fclose(file);
}

void CSinkBase::SeekFile_( const long int offset, const int origin )
{
	// adjust file position
	if (fseek(file,offset,origin))
		throw(runtime_error("Failed to seek the output file."));
		
	// adjust the # of bytes written
	nbytes_total = ftell(file);
}

size_t CSinkBase::ReadFile_(void* buf, const size_t N)
{
	size_t bcount = (size_t) fread((unsigned char *)buf, 1, N, file);
	if (!bcount && ferror(file))
		throw(runtime_error("Failed to read the output file."));
	
	return bcount;
}

size_t CSinkBase::WriteFile_(const void *buf, const size_t N)
{
	// write data
	size_t bcount = (size_t) fwrite((unsigned char*)buf, 1, N, file);
	if (!bcount && ferror(file))
		throw(runtime_error("Failed to write to the output file."));
		
	// update the # of bytes written
	nbytes_total += bcount;
	return bcount;
}

bool CSinkBase::EOF_()
{
	return feof(file);
}

size_t CSinkBase::GetNumberOfBytesWritten_()
{
	return nbytes_total;
}

