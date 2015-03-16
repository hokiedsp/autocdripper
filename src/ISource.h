#pragma once

/**
 * @brief Interface for CDDA device
 */
class ISource
{
    virtual void SetSkipFirstTrackPreGap(const bool val)=0;
    virtual bool GetSkipFirstTrackPreGap()=0;

    /** Get device path
     *
     *  @return Device path string.
     */
    virtual std::string GetPath() const=0;

    virtual std::size_t GetSectorSize()=0; /* number of samples returned by ReadNextSector*/

    /** Get the length of the CD in specified units
     *
     *  @param[in] Output time units (default: sectors)
     *  @return    Length of the CD in specified time unit
     */
    virtual size_t GetLength(cdtimeunit_t units=CDTIMEUNIT_SECTORS)=0;

    virtual const int16_t* ReadNextSector()=0; /* returns non-NULL until end of CD */

    virtual void Rewind()=0; /* rewind to the first sector of the disc*/

};
