#pragma once

#include <vector>
#include <functional>
#include <stdint.h>

#include "SCueSheet.h"

class ISink;
typedef std::vector<ISink*> ISinkPtrVector;
typedef std::vector<std::reference_wrapper<ISink>> ISinkRefVector ;


class ISink
{
public:

    /**
     * @brief IsLocked
     * @return true if ISink instance is currently locked
     */
    virtual bool IsLocked()=0;

    /**
     * @brief Lock the instance. Blocks the calling thread until unlocked
     *        if already locked
     * @param A unique signature (e.g., calling thread manager's address)
     */
    virtual void Lock(const uintptr_t sign)=0;

    /**
     * @brief Try to lock the instance. If already locked, returns false.
     * @param A unique signature (e.g., calling thread manager's address)
     * @return true if successfully locked
     */
    virtual bool TryLock(const uintptr_t sign)=0;

    /**
     * @brief Unlock the instance, given valid signature
     * @param A unique signature (e.g., calling thread manager's address)
     * @return true if successfully unlocked
     */
    virtual bool Unlock(const uintptr_t sign)=0;

    /**
     * @brief Blocks the calling thread until the instance is unlocked by
     *        another thread.
     */
    virtual void WaitTillUnlock()=0;

    /**
     * @brief Write preamble data to the audio file. Derived class must
     *        the access to the data to be written. The ISink instance
     *        must be locked by the calling thread prior to calling this
     *        function.
     * @param A unique signature used to lock the sink
     */
    virtual void WritePreamble(const uintptr_t sign) = 0;

    /**
     * @brief Write a frame of audio data. The ISink instance must be locked
     *        by the calling thread prior to calling this function.
     * @param data buffer (16-bit integer)
     * @param buffer size in the number of samples
     * @param A unique signature used to lock the sink
     * @return Number of samples written
     */
    virtual int WriteFrame(const int16_t* data, const size_t framesize, const uintptr_t sign) = 0;

    /**
     * @brief Write postamble to the output audio file. Derived class must
     *        the access to the data to be written.
     * @param A unique signature used to lock the sink
     */
    virtual void WritePostamble(const uintptr_t sign) = 0;

    /**
     * @brief returns true if cuesheet can be embedded
     * @return true if cuesheet can be embedded
     */
    virtual bool CueSheetEmbeddable()=0;

    /**
     * @brief Add/overwrite "cuesheet" tag entry to the output file.
     * @param[in] reference to the cuesheet
     * @throw std::runtime_error if ISink instance does not support embedded
     *        cuesheet (i.e., CueSheetEmbeddable() returns false)
     */
    virtual void SetCueSheet(const SCueSheet& cuesheet)=0;

};
