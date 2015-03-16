#pragma once

#include <string>
#include <vector>

#include "enums.h"
#include "CGKeyFileBase.h"

#ifndef DEFAULT_MUSIC_DIR
#define DEFAULT_MUSIC_DIR "$HOME/Music"
#endif

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
     * @brief Get General::DrivePath option
     * @return default CD drive path
     */
    std::string GeneralDrivePath();

    /**
     * @brief Get General::FileFormat option
     * @return Output file format as ReleaseDatabase enum type
     */
    OutputFileFormat GeneralFileFormat();

    /**
     * @brief Get General::DoNotRipUnknownDisc option
     * @return true to skip disc if its information is not found in databases
     */
    bool GeneralDoNotRipUnknownDisc();

    /**
     * @brief Get General::OutputDir option
     * @return the base path to place the output file
     */
    std::string GeneralOutputDir();

    /**
     * @brief Get General::FileNamingScheme option
     * @return output file naming scheme if CD info is successfully retrieved
     */
    std::string GeneralFileNamingScheme();

    /**
     * @brief Get General::FileNamingSchemeNoInfo option
     * @return output file naming scheme if CD info is not found
     */
    std::string GeneralFileNamingSchemeNoInfo();

    /**
     * @brief Get General::PromptUPC option
     * @return true to prompt user for the UPC barcode of the album
     */
    bool GeneralPromptUPC();

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
    bool GeneralShowNotification();

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
