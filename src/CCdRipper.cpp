#include "CCdRipper.h"

#include <iostream>
using std::cout;
using std::endl;

using std::vector;

CCdRipper::CCdRipper(ISourceCdda& src, ISink& snk) : source(src), canceled(false)
{
    sinks.emplace_back(snk);
}

CCdRipper::CCdRipper(ISourceCdda& src, const ISinkVector &snks)
    : source(src), sinks(snks) {}

CCdRipper::~CCdRipper() {}

void CCdRipper::ThreadMain()
{
    canceled = false;

    uintptr_t sign = reinterpret_cast<uintptr_t>(this);
    ISinkVector::iterator it;

    // Lock sinks so other threads cannot write to the file while ripping
    for (it = sinks.begin(); it!=sinks.end(); it++)
        (*it).get().Lock(sign);

    try
    {
        // Rip now!
        size_t framesize = source.GetSectorSize();
        const int16_t* data = source.ReadNextSector(); /* returns non-NULL until end of CD */

        while (data && ~stop_request)
        {
            // Write data to all sinks
            for (it = sinks.begin(); it!=sinks.end(); it++)
                (*it).get().WriteFrame(data, framesize, sign);

            // Read next sector
            data = source.ReadNextSector(); /* returns non-NULL until end of CD */
        }

        // if operatio is canceled
        if (data) canceled = true;
    }
    catch (...)
    {
        // Unlock sinks and rethrow the exception
        for (it = sinks.begin(); it!=sinks.end(); it++)
            (*it).get().Unlock(sign);
        throw;
    }

    // Unlock
    for (it = sinks.begin(); it!=sinks.end(); it++)
        (*it).get().Unlock(sign);
}
