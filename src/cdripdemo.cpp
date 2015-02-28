//#define WAVSINK

#include <stdio.h>
#include <exception>

#include "CSourceCdda.h"

#ifdef WAVSINK
#include "CSinkWav.h"
#else
#include "CSinkWavPack.h"
#endif

using std::exception;

int main(int argc, const char *argv[])
{
	const int16_t *p_readbuf;
	size_t framesize;
	
	try
	{
		CSourceCdda cdrom; // auto-detect CD-ROM drive with a audio CD
#ifdef WAVSINK
		CSinkWav wavwriter("track1s.wav"); // save to the wav file
#else
		CSinkWavPack wvwriter("track1s.wv"); // save to the wav file
#endif
		framesize = cdrom.GetSectorSize();
		
#ifdef WAVSINK
		wavwriter.WritePreamble();  // write the preamble of the destination audio file
#else
		wvwriter.WritePreamble();  // write the preamble of the destination audio file
#endif

		while ((p_readbuf = cdrom.ReadNextSector()))
		//for (int i=0;i<50000;i++)
		{
			//p_readbuf = cdrom.ReadNextSector();
#ifdef WAVSINK
			wavwriter.WriteFrame(p_readbuf, framesize);
#else
			wvwriter.WriteFrame(p_readbuf, framesize);
#endif
		}
		
#ifdef WAVSINK
		wavwriter.WritePostamble();	// go back to header and fill the size
#else
		wvwriter.WritePostamble();	// go back to header and fill the size
#endif
	}
	catch (exception& e)
	{
		printf("%s\n",e.what());
		return 1;
	}

	return 0;
}

