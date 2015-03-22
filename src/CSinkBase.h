#pragma once

#include <cstdint>
#include <string>
#include <mutex>
#include <condition_variable>

#include "ISink.h"

class CSinkBase : public ISink
{
public:
	CSinkBase(const std::string &path);
	virtual ~CSinkBase();

    virtual bool IsLocked();
    virtual void Lock(const uintptr_t sign);
    virtual bool TryLock(const uintptr_t sign);
    virtual bool Unlock(const uintptr_t sign);
    virtual void WaitTillUnlock();

protected:

    virtual void SeekFile_(const long int offset, const int origin);
	virtual size_t ReadFile_(void* buf, const size_t N);
	virtual size_t WriteFile_(const void *buf, const size_t N);
	virtual bool EOF_(); // returns true if end-of-file
	virtual size_t GetNumberOfBytesWritten_();

    virtual uintptr_t GetLockSign_();

private:

    std::condition_variable cv_sign; // condition variable to wait for unlock
    std::mutex mutex_sign; // mutex to protect lock_sign
    uintptr_t lock_sign;   // lock signature

	FILE* file;
	size_t nbytes_total;
};

