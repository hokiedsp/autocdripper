#pragma once

#include <string>
#include <vector>

#include "enums.h"
#include "CGKeyFileBase.h"

class CGKeyFileAutoCDRipper : public CGKeyFileBase
{
public:
    /**
     * @brief Constructor
     * @param[in] the path to the key file
     */
    CGKeyFileAutoCDRipper(const std::string &filename);

    /**
     * @brief Destructor
     */
    virtual ~CGKeyFileAutoCDRipper();

    /**
     * @brief Get General::SkipUnknownDisc option
     * @return true to skip disc if its information is not found in databases
     */
    bool GeneralSkipUnknownDisc();

    /**
     * @brief Get General::FileFormat option
     * @return Output file format as ReleaseDatabase enum type
     */
    OutputFileFormat GeneralFileFormat();

    /**
     * @brief Get General::SkipTrackOnePregap option
     * @return true to skip the first tracks' pregap
     */
    bool GeneralSkipTrackOnePreGap();

    /**
     * @brief Get General::DatabasePreferenceList option
     * @return database list (ReleaseDatabase enum list)
     */
    std::vector<ReleaseDatabase> GeneralDatabasePreferenceList();

    /**
     * @brief Get General::CoverArt option
     * @return true to embed cover art
     */
    bool GeneralCoverArt();

    /**
     * @brief Get General::CoverArtPreferredSize option
     * @return preferred dimension of the coverart image in pixels
     */
    int GeneralCoverArtPreferredSize();

    /**
     * @brief Get General::ShowNotification option
     * @return true to enable notification
     */
    int GeneralShowNotification();

    /**
     * @brief Get Rems::DBINFO option
     * @return true to embed databse info (name and disc/release ID)
     */
    bool RemsDbInfo();

    /**
     * @brief Get Rems::DATE option
     * @return true to embed release date
     */
    bool RemsDate();

    /**
     * @brief Get Rems::LABEL option
     * @return true to embed cd label name
     */
    bool RemsLabel();

    /**
     * @brief Get Rems::COUNTRY option
     * @return true to embed country of origin
     */
    bool RemsCountry();

    /**
     * @brief Get Rems::UPC option
     * @return true to embed UPC barcode numbers
     */
    bool RemsUPC();

    /**
     * @brief Get Rems::ASIN option
     * @return true to embed Amazon product code
     */
    bool RemsASIN();

protected:
    /**
     * @brief Generate list of supported output file formats
     * @return list of file formats
     */
    std::vector<std::string> GenerateOutputFileFormatList();

    /**
     * @brief Generate list of supported databases
     * @return list of databases
     */
    std::vector<std::string> ReleaseDatabaseList();


    /**
     * @brief Get General::DatabasePreferenceList option
     * @return database list (string list)
     */
    std::vector<std::string> GeneralDatabasePreferenceStringList();

private:
    /** Create new key_file object with default values
     *
     * @brief Initialize an empty key file
     */
    void Initialize_();


};
