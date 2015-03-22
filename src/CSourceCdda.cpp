#include "CSourceCdda.h"

#include <stdexcept>
#include <iostream>

/*
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cdio/paranoia.h>
*/
#include <cdio/cd_types.h>
#include <cdio/mmc.h>
//#include <cdio/util.h>

using std::string;
using std::runtime_error;

CSourceCdda::CSourceCdda()	// auto-detect CD-ROM drive
: d(NULL)
{
	OpenDisc_();	// throws exception if failed
    try
    {
        InitParanoia_(); // throws exception if failed
    }
    catch (...)
    {
        cdda_close(d);
        throw;
    }
}

CSourceCdda::CSourceCdda(const std::string &path) // use the given drive
: d(NULL)
{
	OpenDisc_(path.c_str());	// throws exception if failed
    try
    {
        InitParanoia_(); // throws exception if failed
    }
    catch (...)
    {
        cdda_close(d);
        throw;
    }
}

CSourceCdda::~CSourceCdda()
{
	paranoia_free(p);
	cdda_close(d);
}

void CSourceCdda::OpenDisc_(const char * drivepath)
{
	const char **ppsz_search_drive = NULL;
	char **ppsz_cd_drives; /* List of all drives with a loaded CDDA in it. */

	/* if drive path is given, specifically look for it */
	if (drivepath!=NULL)
	{
		ppsz_search_drive = new const char* [2];
		ppsz_search_drive[0] = drivepath;
		ppsz_search_drive[1] = NULL;
	}

	/* See if we can find a device with a loaded CD-DA in it. */
	ppsz_cd_drives = cdio_get_devices_with_cap(const_cast<char**>(ppsz_search_drive), CDIO_FS_AUDIO, false);

	if (drivepath!=NULL) delete[] ppsz_search_drive; // done with the search list
	
	if (!(ppsz_cd_drives && *ppsz_cd_drives)) // no disc found
	{
		if (ppsz_cd_drives) cdio_free_device_list(ppsz_cd_drives);
		throw (runtime_error("Unable find or access a CD-ROM drive with an audio CD in it."));
	}

	/* Found such a CD-ROM with a CD-DA loaded. Use the first drive in
	the list. */
	d = cdda_identify(*ppsz_cd_drives, 1, NULL);

    /* Quiet operation */
    cdda_verbose_set(d, CDDA_MESSAGE_FORGETIT, CDDA_MESSAGE_FORGETIT);


	/* Don't need a list of CD's with CD-DA's any more. */
	cdio_free_device_list(ppsz_cd_drives);
	
	/* make sure valid disc object is returned */
	if (!d) throw (runtime_error("Unable to identify audio CD."));

	/* Open the disc */
	if (cdda_open(d))
	{
		cdda_close(d);
		throw (runtime_error("Unable to open audio CD."));
	}

    /* CD is ready. */
}

/** Get device path
 *
 *  @return Device path string.
 */
std::string CSourceCdda::GetDevicePath() const
{
   return string(((cdrom_drive_s*)d)->cdda_device_name);
}

/** Get the length of the CD in specified units
 *  
 *  @param[in] Output time units (default: sectors)
 *  @return    Length of the CD in specified time unit
 */
size_t CSourceCdda::GetLength(cdtimeunit_t units)
{
   /* Now all we still have to do, is calculate the length of the
       disc in requested time units.  We use the LEADOUT_TRACK for this. */
    CdIo_t *cdio = ((cdrom_drive_s*)d)->p_cdio;
    uint32_t length = cdio_get_track_lba(cdio, CDIO_CDROM_LEADOUT_TRACK);

	switch (units)
	{
		case CDTIMEUNIT_SECONDS:
			return length/CDIO_CD_FRAMES_PER_SEC;
		case CDTIMEUNIT_SECTORS:
			return length;
		case CDTIMEUNIT_WORDS:
			return length*(CDIO_CD_FRAMESIZE_RAW/2);
		default: //case CDTIMEUNIT_BYTES:
			return length*CDIO_CD_FRAMESIZE_RAW;
	}
}

void CSourceCdda::InitParanoia_()
{
	p = paranoia_init(d);
	paranoia_modeset(p, PARANOIA_MODE_FULL^PARANOIA_MODE_NEVERSKIP);
    i_last_lsn = cdda_disc_lastsector(d);
    Rewind();
}
	
size_t CSourceCdda::GetSectorSize()
{
	return CDIO_CD_FRAMESIZE_RAW/2;
}

const int16_t* CSourceCdda::ReadNextSector()
{
	/* return NULL if reached the end */
	if (i_curr_lsn>i_last_lsn) return NULL;

	/* read a sector */
	int16_t *p_readbuf = paranoia_read(p, NULL);
	if(!p_readbuf) throw (runtime_error("paranoia read error. Stopping."));

	/* increment the sector counter */
	i_curr_lsn++;
	
	return p_readbuf;
}

void CSourceCdda::Rewind()
{
	/* Set reading mode for full paranoia, but allow skipping sectors. */
    lsn_t lsn = paranoia_seek(p, 0, SEEK_SET);
	i_curr_lsn = 0;
}

/** /brief Fill track info on SCueSheet Cd object
 * 
 */
SCueSheet CSourceCdda::GetCueSheet() /* populates CueSheet */
{
    SCueSheet CueSheet;

	// initialize tracks
	CueSheet.AddTracks(cdda_tracks(d));

	// check pregap for Track 1. If valid sector returned, 
	// create the first track on the cuesheet and set its a pregap index
	if (cdda_track_lastsector(d,0)>=0)
        CueSheet.Tracks[0].AddIndex(0,cdda_track_firstsector(d,0));
	else
		CueSheet.Tracks[0].AddIndex(0,0);

	bool mmc_valid = true;

	/* grab the cdio object */
	CdIo_t* p_cdio = ((cdrom_drive_s*)d)->p_cdio;

	/* get and save MCN */
  	char* mcn_str = cdio_get_mcn(p_cdio);
  	if (mcn_str)
  	{
  		CueSheet.Catalog = mcn_str;
  		if (!CueSheet.CheckCatalog()) 
  		{
  			CueSheet.Catalog.clear();	// make sure its valid
  			mmc_valid = false;
  		}
		free(mcn_str);
	}
	else
	{
		mmc_valid = false;
	}

	// for each track
	for (size_t i = 1; i <= cdda_tracks(d) ; i++)
	{
		// record the starting index
        size_t start = cdda_track_firstsector(d,i);	// add unaccounted 150 pregap sectors

		// get the track object
		SCueTrack &track = CueSheet.Tracks[i-1];

		// add Index 1 with the start time
		track.AddIndex(1,start);
		
		// look for ISRC
		char isrc_str[13] = {};
		if (mmc_valid && !mmc_isrc_track_read_subchannel (p_cdio, i, isrc_str) && isrc_str[0])
		{
			track.ISRC = isrc_str;
			if (!track.CheckISRC())
			{
				track.ISRC.clear();
				mmc_valid = false;
			}
			//free(isrc_str);	// for libcdio 0.93
		}
		else
		{
			mmc_valid = false;
		}
	}

    return CueSheet;
}
