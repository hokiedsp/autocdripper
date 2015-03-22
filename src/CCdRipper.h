#pragma once

#include "ISourceCdda.h"
#include "ISink.h"

#include "CThreadManBase.h"

class CCdRipper : public CThreadManBase
{
public:
    CCdRipper(ISourceCdda& source, ISink& sink);
    CCdRipper(ISourceCdda& source, const ISinkVector &sinks);
    virtual ~CCdRipper();

    bool Canceled() const { return canceled; }

protected:
    /**
     * @brief Thread's Main function. Shall be implemented by derived class
     */
    virtual void ThreadMain();

private:
    ISourceCdda &source;
    ISinkVector sinks;
    bool canceled;
};
