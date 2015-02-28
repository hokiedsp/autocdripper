#pragma once

#include <string>
#include <queue>
#include <wavpack/wavpack.h>

#include "CSinkBase.h"
#include "CTagsAPEv2.h"

class CSinkWavPack : CSinkBase
{
public:
	CTagsAPEv2 tags;

	CSinkWavPack(const std::string &path);
	virtual ~CSinkWavPack();

	virtual void WritePreamble();
	virtual int WriteFrame(const int16_t* data, const size_t framesize);
	virtual void WritePostamble();

	virtual size_t WriteFile(const void *buf, const size_t N);

private:
	uint32_t first_block_size;
	
	WavpackConfig config;
	WavpackContext *wpc;
	
	std::vector<int32_t> buffer;
	
	virtual void WriteTags_();
	virtual void UpdatePreamble_();
};

