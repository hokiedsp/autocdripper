#pragma once

#include "ISourceCdda.h"
#include "ISink.h"

#include "CThreadManBase.h"

class CCdRipper : public CThreadManBase
{
public:
    CCdRipper(ISourceCdda& source, ISink& sink);
    CCdRipper(ISourceCdda& source, const ISinkRefVector &sinks);
    virtual ~CCdRipper();

    /**
     * @brief Returns the status of last thread run
     * @return true if its thread was externally stopped prematurely during
     *         its last run.
     */
    bool Canceled() const { return canceled; }

protected:
    /**
     * @brief Thread's Main function. Shall be implemented by derived class
     */
    virtual void ThreadMain();

private:
    ISourceCdda &source;
    ISinkRefVector sinks;
    bool canceled;
};
