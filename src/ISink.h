#pragma once

#include <stdint.h>

class ISink
{
public:
	virtual void WritePreamble() = 0;
	virtual int WriteFrame(const int16_t* data, const size_t framesize) = 0;
	virtual void WritePostamble() = 0;
};

