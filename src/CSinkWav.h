#pragma once

#include "CSinkBase.h"

#include <string>
#include <stdint.h>

class CSinkWav : CSinkBase
{
public:
	CSinkWav(const std::string &path);
	virtual ~CSinkWav();

	virtual void WritePreamble();
	virtual int WriteFrame(const int16_t* data, const size_t framesize);
	virtual void WritePostamble();

private:
	void WriteInteger_(long int num, int n);	// write an integer as an n-byte entity
};

