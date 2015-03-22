#pragma once

#include <string>
#include <queue>
#include <wavpack/wavpack.h>

#include "CSinkBase.h"
#include "CTagsAPEv2.h"

class CSinkWavPack : public CSinkBase
{
public:
	CTagsAPEv2 tags;

	CSinkWavPack(const std::string &path);
	virtual ~CSinkWavPack();

    virtual void WritePreamble(const uintptr_t sign);
    virtual int WriteFrame(const int16_t* data, const size_t framesize, const uintptr_t sign);
    virtual void WritePostamble(const uintptr_t sign);

	virtual size_t WriteFile(const void *buf, const size_t N);

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
	uint32_t first_block_size;
	
	WavpackConfig config;
	WavpackContext *wpc;
	
	std::vector<int32_t> buffer;
	
	virtual void WriteTags_();
	virtual void UpdatePreamble_();
};

