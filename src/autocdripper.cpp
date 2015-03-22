#include <stdio.h>
#include <exception>

#include <iostream>
#include <iomanip>
using std::cout;
using std::endl;

#include "CCdRipper.h"
#include "CSourceCdda.h"
#include "CSinkWav.h"
#include "CSinkWavPack.h"

#include "CDbFreeDb.h"

using std::exception;

int main(int argc, const char *argv[])
{
	try
	{
		CSourceCdda cdrom; // auto-detect CD-ROM drive with a audio CD
        CSinkWav wavwriter("track1s.wav"); // save to the wav file
        CSinkWavPack wvwriter("track1s.wv"); // save to the wav file

        CDbFreeDb freedb;

        freedb.SetCacheSettings("off");
        freedb.Query(cdrom.GetDevicePath(), cdrom.GetCueSheet(), cdrom.GetLength(),true);	// auto-populate

        cout << std::setfill ('0') << std::setw(8) << std::hex << freedb.GetDiscId() << std::dec << endl;
        cout << "Found " << freedb.NumberOfMatches() << " matches found" << endl;
        //int discnum = freedb.MatchByGenre("jazz");
        //cout << "matched disc: " << discnum << std::endl;

        SCueSheet* cs = freedb.Retrieve();
        cout << *cs << endl;
        delete cs;
return 0;
        //ISinkRefVector writers = {wavwriter,wvwriter};
        ISinkRefVector writers = {wavwriter};

        ISinkRefVector::iterator it;
        for (it=writers.begin();it!=writers.end();it++)
        {
            ISink &writer = (*it).get();
            writer.Lock(1);
            writer.WritePreamble(1);  // write the preamble of the destination audio file
            writer.Unlock(1);
        }

        // rip the CD
        cout << "MAIN: Instantiating CCdRipper class\n";
        CCdRipper ripper(cdrom,writers);
        cout << "MAIN: Starting CCdRipper thread\n";
        ripper.Start();
        cout << "MAIN: Waiting till CCdRipper thread completes its task\n";
        ripper.WaitTillDone();

        if (ripper.Canceled())
        {
            // delete the output file
        }
        else
        {
            for (it=writers.begin();it!=writers.end();it++)
            {
                ISink &writer = (*it).get();
                writer.Lock(1);
                writer.WritePostamble(1);	// go back to header and fill the size
                writer.Unlock(1);
            }
        }
        cout << "MAIN: All completed\n";
    }
	catch (exception& e)
	{
		printf("%s\n",e.what());
		return 1;
	}

	return 0;
}
