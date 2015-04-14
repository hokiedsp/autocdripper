#include <stdio.h>
#include <exception>

#include <iostream>
#include <iomanip>
using std::cout;
using std::endl;

#include "CCdRipper.h"
#include "CCueSheetBuilder.h"

#include "CSourceCdda.h"
#include "CSinkWav.h"
#include "CSinkWavPack.h"

#include "CDbFreeDb.h"
#include "CDbMusicBrainz.h"
#include "CDbDiscogs.h"
#include "CDbLastFm.h"

using std::exception;

int main(int argc, const char *argv[])
{
    try
    {
        CDbFreeDb freedb;
        CDbMusicBrainz mbdb;
        CDbDiscogs discogs;
        CDbLastFm lastfm("0691e8527e395f789d23e4e91b0be8fc");
//                const string Secret = "ce74c6a6955a7e9e2a2e6makeed8cfa796a3";
        CCueSheetBuilder csbuilder;

        freedb.SetCacheSettings("off");
        discogs.SetCountryPreference("US");

        CSourceCdda cdrom; // auto-detect CD-ROM drive with a audio CD

        csbuilder.SetCdInfo(cdrom);
        //csbuilder.SetCdInfo(cdrom,"731452547224"); // jobim songbook
        //csbuilder.SetCdInfo(cdrom,"025218643429"); // bill evans moon beams

        //csbuilder.AddDatabase(discogs);
        csbuilder.AddDatabase(mbdb);
        //csbuilder.AddDatabase(lastfm);
        //csbuilder.AddDatabase(freedb);

        csbuilder.AddRemField(AlbumRemFieldType::DBINFO);
        csbuilder.AddRemField(AlbumRemFieldType::UPC);
        csbuilder.AddRemField(AlbumRemFieldType::DISC);
        csbuilder.AddRemField(AlbumRemFieldType::DISCS);
        csbuilder.AddRemField(AlbumRemFieldType::GENRE);
        csbuilder.AddRemField(AlbumRemFieldType::LABEL);
        csbuilder.AddRemField(AlbumRemFieldType::CATNO);
        csbuilder.AddRemField(AlbumRemFieldType::COUNTRY);
        csbuilder.AddRemField(AlbumRemFieldType::DATE);

        csbuilder.RequireUpcMatch(false);
        csbuilder.AllowCombinig(true, false);

        cout << "[MAIN] Starting CCueSheetBuilder thread\n";
        csbuilder.Start();
        cout << "[MAIN] Waiting till CCueSheetBuilder thread completes its task\n";
        csbuilder.WaitTillDone();

        if (csbuilder.FoundRelease())
        {
            cout << "[MAIN] Retrieving the populated cuesheet...\n";
            SCueSheet cs = csbuilder.GetCueSheet();
            cout << cs << endl;
        }
        else
            cout << "[MAIN] CD info was not found online\n";

        /*
        CSinkWav wavwriter("track1s.wav"); // save to the wav file
        CSinkWavPack wvwriter("track1s.wv"); // save to the wav file
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
        */
    }
    catch (exception& e)
    {
        printf("%s\n",e.what());
        return 1;
    }

    return 0;
}
