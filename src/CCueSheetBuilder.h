#prgma once

#include "CThreadManBase.h"

#include "CSourceCdda.h"
#include "SCueSheet.h"
#include "enums.h"
#include "IDatabase.h"
#include "CDbCDDB.h"
#include "CDbMusicBrainz.h"
#include "CDbDiscogs.h"
#include "CDbLastFm.h"

/**
 * @brief The CCueSheetBuilder class
 */
class CCueSheetBuilder : public CThreadManBase
{
public:
    CCueSheetBuilder();
    virtual ~CCueSheetBuilder();

    bool Canceled() const { return canceled; }

    void SetCdInfo(const CSourceCdda &cdrom, std::string upc="");
    void AddDatabase(ReleaseDatabase dbtype, const std::string &userkey="");

    /**
     * @brief Get generated cuesheet
     * @return Read-only reference to the internal cuesheet
     */
    const SCueSheet &GetCueSheet() const;

    std::vector<unsigned char> FrontData() const;

protected:
    /**
     * @brief Thread's Main function. Shall be implemented by derived class
     */
    virtual void ThreadMain();

private:
    bool canceled;
    bool request_userkey;
    SCueSheet CueSheet;

    std::vector<CDbBase*> databases;

};
