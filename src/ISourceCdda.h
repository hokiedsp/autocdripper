#pragma once

#include <string>
#include <vector>
#include <functional>

class ISourceCdda;
typedef std::vector<ISourceCdda*> ISourceCddaPtrVector;
typedef std::vector<std::reference_wrapper<ISourceCdda>> ISourceCddaRefVector;

#include "SCueSheet.h"

/**
 * @brief Interface for CDDA device
 */
class ISourceCdda
{
public:
    /** Get device path (read-only)
     *
     *  @return Device path string.
     */
    virtual std::string GetDevicePath() const=0;

    /**
     * @brief Get read buffer size (read-only)
     * @return Number of samples returned by ReadNextSector()
     */
    virtual size_t GetSectorSize() const=0;

    /**
     * @brief ReadNextSector
     * @return a pointer to internal buffer. No need to be deallocated. The buffer
     *         content is valid until the subsequent ReadNextSector() call. Returns
     *         NULL if all sectors have been written.
     */
    virtual const int16_t* ReadNextSector()=0; /* returns non-NULL until end of CD */

    /**
     * @brief Rewind to the beginning of the disc.
     */
    virtual void Rewind()=0; /* rewind to the first sector of the disc*/

    /** Get the length of the CD in specified units
     *
     *  @param[in] Output time units (default: sectors)
     *  @return    Length of the CD in specified time unit
     */
    virtual size_t GetLength(cdtimeunit_t units=CDTIMEUNIT_SECTORS) const=0;

    /**
     * @brief Get a cuesheet object populated with the CD track info
     * @return SCueSheet instance with fully populated track information.
     */
    virtual SCueSheet GetCueSheet() const=0;

};
