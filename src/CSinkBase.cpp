#include "CSinkBase.h"

#include <stdexcept>
#include <cstdio>

using std::string;
using std::runtime_error;
using std::mutex;
using std::lock_guard;
using std::unique_lock;

CSinkBase::CSinkBase(const std::string &path) : lock_sign(0), nbytes_total(0)
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

bool CSinkBase::IsLocked()
{
    lock_guard<mutex> lck(mutex_sign);
    return lock_sign;
}

void CSinkBase::Lock(const uintptr_t sign)
{
    unique_lock<mutex> lck(mutex_sign);
    // if locked, block until unlocked
    while (lock_sign) cv_sign.wait(lck);

    lock_sign = sign;
}

bool CSinkBase::TryLock(const uintptr_t sign)
{
    lock_guard<mutex> lck(mutex_sign);
    if (lock_sign) return false;

    lock_sign = sign;
    return true;
}

bool CSinkBase::Unlock(const uintptr_t sign)
{
    std::unique_lock<std::mutex> lck(mutex_sign);

    if (lock_sign!=sign) return false;

    lock_sign = 0;
    cv_sign.notify_all();
    return true;
}

void CSinkBase::WaitTillUnlock()
{
    unique_lock<mutex> lck(mutex_sign);
    // if locked, block until unlocked
    while (lock_sign) cv_sign.wait(lck);
}

uintptr_t CSinkBase::GetLockSign_()
{
    lock_guard<mutex> lck(mutex_sign);
    return lock_sign;
}
