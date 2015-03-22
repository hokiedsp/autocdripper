#pragma once

#include "CSinkBase.h"

#include <string>
#include <stdint.h>

class CSinkWav : public CSinkBase
{
public:
	CSinkWav(const std::string &path);
	virtual ~CSinkWav();

    virtual void WritePreamble(const uintptr_t sign);
    virtual int WriteFrame(const int16_t* data, const size_t framesize, const uintptr_t sign);
    virtual void WritePostamble(const uintptr_t sign);

    /**
     * @brief returns true if cuesheet can be embedded
     * @return true if cuesheet can be embedded
     */
    virtual bool CueSheetEmbeddable();

    /**
     * @brief Add "cuesheet" tag entry to the output file
     * @param[in] reference to the cuesheet
     * @throw std::runtime_error if ISink instance does not support embedded
     *        cuesheet (i.e., CueSheetEmbeddable() returns false)
     */
    virtual void SetCueSheet(const SCueSheet& cuesheet);
private:
	void WriteInteger_(long int num, int n);	// write an integer as an n-byte entity
};

