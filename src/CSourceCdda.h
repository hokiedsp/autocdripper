#pragma once

#include <string>

#include <cinttypes>
//#include <sys/types.h>

#include <cdio/paranoia.h>

#include "ISourceCdda.h"

class CSourceCdda : public ISourceCdda
{
public:
	/** Constructor.
	 */
	CSourceCdda();
	
	/** Constructor with device path.
	 */
	CSourceCdda(const std::string &path);
	
	/** Destructor
	 */
	~CSourceCdda();

	/** Get device path
	 *
	 *  @return Device path string.
	 */
    std::string GetDevicePath() const;

    size_t GetSectorSize() const; /* number of samples returned by ReadNextSector*/

	/** Get the length of the CD in specified units
	 *  
	 *  @param[in] Output time units (default: sectors)
	 *  @return    Length of the CD in specified time unit
	 */
    size_t GetLength(cdtimeunit_t units=CDTIMEUNIT_SECTORS) const;
	
	const int16_t* ReadNextSector(); /* returns non-NULL until end of CD */
	void Rewind(); /* rewind to the first sector of the disc*/
	
    /**
     * @brief Get a cuesheet object populated with the CD track info
     * @return SCueSheet instance with fully populated track information.
     */
    virtual SCueSheet GetCueSheet() const;
	
private:
	cdrom_drive_t *d; 	/* Place to store handle given by cd-paranoia. */
	cdrom_paranoia_t *p; /* Place to store paranoia object. */

	lsn_t i_curr_lsn; 			/* current LSN */
	lsn_t i_last_lsn;				/* last LSN */
	
	void OpenDisc_(const char * path=NULL);
	void InitParanoia_();
	void CloseDisc_();

	
};

