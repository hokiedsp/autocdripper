#include "CSinkWav.h"

#include <stdexcept>

using std::string;
using std::runtime_error;

CSinkWav::CSinkWav(const string &path) :	CSinkBase(path)
{}	

CSinkWav::~CSinkWav()
{}

void CSinkWav::WriteInteger_(long int num, int bytes)
{
	unsigned int i;
	unsigned char c;
	for (i=0; bytes--; i++)
	{
		c = (num >> (i<<3)) & 0xff;
		WriteFile_(&c, 1);
	}
}

#define WriteString_(s) \
WriteFile_(s, sizeof(s)-1) /* Subtract 1 for trailing '\0'. */

/* Write a the header for a WAV file. */
void CSinkWav::WritePreamble()
{
	/* quick and dirty */
	WriteString_("RIFF"); /* 0-3 : FILE ID String */

	SeekFile_(4, SEEK_CUR);	// skip the file size for now
	
	WriteString_("WAVEfmt "); /* 8-15 : File type header + format chunk header */
	WriteInteger_(16, 4); /* 16-19 : length of the header so far*/
	WriteInteger_(1,  2); /* 20-21 : type of format */
	WriteInteger_(2, 2); /* 22-23 : number of channels*/
	WriteInteger_(44100, 4); /* 24-27 : sample rate */
	WriteInteger_(44100*2*2, 4); /* 28-31 : total bitrate in bytes/second */
	WriteInteger_(4, 2); /* 32-33 : total # of bytes per sample (all channels)*/
	WriteInteger_(16, 2); /* 34-35 : bits per sample */
	WriteString_("data"); /* 36-39 : data chunk header */
}

int CSinkWav::WriteFrame(const int16_t* data, const size_t framesize)
{
	return WriteFile_(data, 2*framesize);
}

void CSinkWav::WritePostamble()
{
	size_t nbytes_total = GetNumberOfBytesWritten_();
	SeekFile_(4, SEEK_SET);	// skip the file size for now
	WriteInteger_(nbytes_total-8, 4); /* 4-7 : Size of the overall file - 8*/
	SeekFile_(40, SEEK_SET);	// skip the file size for now
	WriteInteger_(nbytes_total-44, 4); /* 40-43 : data chunk size*/
	SeekFile_(0, SEEK_END);	// move the cursor to the end
}

