//#define WAVSINK

#include <iostream>
#include <iomanip>
#include <exception>
#include <string>
#include <memory>

#include "CSourceCdda.h"
//#include "CDbCDDB.h"
#include "CDbMusicBrainz.h"

using std::string;
using std::exception;
using std::cout;
using std::endl;

int main(int argc, const char *argv[])
{
	try
	{
		CSourceCdda cdrom; // auto-detect CD-ROM drive with a audio CD
		/*
		CDbCDDB cddb;
		
		cddb.SetCacheSettings("off");
		cddb.Query(cdrom.GetPath(), cdrom.CueSheet, cdrom.GetLength(),true);	// auto-populate
		
		cout << std::setfill ('0') << std::setw(8) << std::hex << cddb.GetDiscId() << std::dec << endl;
		
		int discnum = cddb.MatchByGenre("jazz"); 
		cout << "matched disc: " << discnum << std::endl;
		
		if (discnum<0) cddb.Print();	// disc not found in the genre, print the disc info
		else
		{
			std::unique_ptr<SDbrBase> cdr(cddb.Retrieve(discnum));
			cout << *cdr << std::endl;	// retrieve the CueSheet, print
		}
		*/
		
		CDbMusicBrainz mb5;
		mb5.Query(cdrom.GetPath(), cdrom.CueSheet, cdrom.GetLength(),false);	// auto-populate
		cout << "DISCID: " << mb5.GetDiscId() << endl << endl;
		mb5.Print();
		int nmatches = mb5.NumberOfMatches();
		if (nmatches>0)
		{
			mb5.Populate(nmatches-1);
			//mb5.Print(nmatches-1);
			std::unique_ptr<SDbrBase> cdr(mb5.Retrieve(nmatches-1));
			cout << *cdr << endl;
		}
	}
	catch (exception& e)
	{
		cout << e.what() << endl;
		return 1;
	}

	return 0;
}

