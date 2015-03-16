#include "CGKeyFileAutoCDRipper.h"

#include <stdexcept>
#include <memory>

#include <iostream>

using std::runtime_error;
using std::string;

///////////////////////////////////////////////////////////////////////////////////////////////


/**
 * @brief Constructor
 * @param[in] the path to the key file
 */
CGKeyFileAutoCDRipper::CGKeyFileAutoCDRipper(const std::string &filename) : CGKeyFileBase(filename)
{
    // if file does not exist, create a new one
    if (key_file==NULL)
    {
        // create a new CONF file
        key_file = g_key_file_new();

        // Key file needs to be created. Initialize key_file object to the initial state
        // for the derived class
        Initialize_();

        // Immediately save the file
        Save();
    }
}

/**
 * @brief Destructor
 */
CGKeyFileAutoCDRipper::~CGKeyFileAutoCDRipper() {}

///////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Get General::DrivePath option
 * @return default CD drive path, empty string indicates auto-detect
 */
std::string CGKeyFileAutoCDRipper::GeneralDrivePath()
{
    return GetStringKey("General","DrivePath","");
}

/**
 * @brief Get General::FileFormat option
 * @return Output file format as ReleaseDatabase enum type
 */
OutputFileFormat CGKeyFileAutoCDRipper::GeneralFileFormat()
{
    return GetStringKey<OutputFileFormat>("General", "FileFormat",
                                          OutputFileFormat::WAVPACK, sttooff);
}


/**
 * @brief Get General::OutputDir option
 * @return the base path to place the output file
 */
std::string CGKeyFileAutoCDRipper::GeneralOutputDir()
{
    return GetStringKey("General", "OutputDir", DEFAULT_MUSIC_DIR);
}

/**
 * @brief Get General::FileNamingScheme option
 * @return returns output file naming scheme if CD info is successfully retrieved
 */
std::string CGKeyFileAutoCDRipper::GeneralFileNamingScheme()
{
    return GetStringKey("General","FileNamingScheme",
                        "%album artist%-%album%[ (%discnumber% of %totaldiscs%)]");
}

/**
 * @brief Get General::DoNotRipUnknownDisc option
 * @return true to skip disc if its information is not found in databases
 */
bool CGKeyFileAutoCDRipper::GeneralDoNotRipUnknownDisc()
{
    return GetBooleanKey("General", "DoNotRipUnknownDisc", true);
}

/**
 * @brief Get General::FileNamingSchemeNoInfo option
 * @return returns output file naming scheme if CD info is not found
 */
std::string CGKeyFileAutoCDRipper::GeneralFileNamingSchemeNoInfo()
{
    return GetStringKey("General","FileNamingScheme",
                        "%cddbid%");
}

/**
 * @brief Get General::PromptUPC option
 * @return true to prompt user for the UPC barcode of the album
 */
bool CGKeyFileAutoCDRipper::GeneralPromptUPC()
{
    return GetBooleanKey("General", "GeneralPromptUPC", false);
}

/**
 * @brief Get General::SkipTrackOnePreGap option
 * @return true to skip the first tracks' pregap
 */
bool CGKeyFileAutoCDRipper::GeneralSkipTrackOnePreGap()
{
    return GetBooleanKey("General", "SkipTrackOnePreGap", true);
}

/**
 * @brief Get General::DatabasePreferenceList option
 * @return database list (string list)
 */
std::vector<std::string> CGKeyFileAutoCDRipper::GeneralDatabasePreferenceStringList()
{
    // Get the enum list (to validate the value)
    std::vector<ReleaseDatabase> elist = GeneralDatabasePreferenceList();

    // Convert enum value to string value
    std::vector<std::string> rlist;
    rlist.reserve(elist.size());
    for (std::vector<ReleaseDatabase>::iterator it = elist.begin();it!=elist.end();it++)
        rlist.push_back(to_string(*it));
    return rlist;
}

/**
 * @brief Get General::DatabasePreferenceList option
 * @return database list (ReleaseDatabase enum list)
 */
std::vector<ReleaseDatabase> CGKeyFileAutoCDRipper::GeneralDatabasePreferenceList()
{
    // create default vector
    std::vector<ReleaseDatabase> defval;
    defval.reserve(4);
    defval.push_back(ReleaseDatabase::DISCOGS);
    defval.push_back(ReleaseDatabase::MUSICBRAINZ);
    defval.push_back(ReleaseDatabase::FREEDB);
    defval.push_back(ReleaseDatabase::LASTFM);

    return GetStringListKey<ReleaseDatabase>("General", "DataBasePreferenceList",
                                             defval, sttordb);
}

/**
 * @brief Get General::CoverArt option
 * @return true to embed cover art
 */
bool CGKeyFileAutoCDRipper::GeneralCoverArt()
{
    return GetBooleanKey("General", "CoverArt", true);
}

/**
 * @brief Get General::CoverArtPreferredSize option
 * @return preferred dimension of the coverart image in pixels
 */
int CGKeyFileAutoCDRipper::GeneralCoverArtPreferredSize()
{
    return GetIntegerKey("General", "CoverArtPreferredSize",300);
}

/**
 * @brief Get General::ShowNotification option
 * @return true to enable notification
 */
bool CGKeyFileAutoCDRipper::GeneralShowNotification()
{
    return GetBooleanKey("General", "ShowNotification", true);
}

/**
 * @brief Get Rems::DBINFO option
 * @return true to embed databse info (name and disc/release ID)
 */
bool CGKeyFileAutoCDRipper::RemsDbInfo()
{
    return GetBooleanKey("Rems", "DBINFO", true);
}

/**
 * @brief Get Rems::DATE option
 * @return true to embed release date
 */
bool CGKeyFileAutoCDRipper::RemsDate()
{
    return GetBooleanKey("Rems", "DATE", true);
}


/**
 * @brief Get Rems::LABEL option
 * @return true to embed cd label name
 */
bool CGKeyFileAutoCDRipper::RemsLabel()
{
    return GetBooleanKey("Rems", "LABEL", true);
}


/**
 * @brief Get Rems::COUNTRY option
 * @return true to embed country of origin
 */
bool CGKeyFileAutoCDRipper::RemsCountry()
{
    return GetBooleanKey("Rems", "COUNTRY", true);
}


/**
 * @brief Get Rems::UPC option
 * @return true to embed UPC barcode numbers
 */
bool CGKeyFileAutoCDRipper::RemsUPC()
{
    return GetBooleanKey("Rems", "UPC", true);
}

///////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Generate list of supported output file formats
 * @return comma-separated list of file formats
 */
std::vector<std::string> CGKeyFileAutoCDRipper::GenerateOutputFileFormatList()
{
    std::vector<std::string> rval;
    rval.reserve(NumberOfOutputFileFormats);

    for (int n = 0; n<NumberOfOutputFileFormats; n++)
        rval.push_back(to_string(static_cast<OutputFileFormat>(n)));

    return rval;
}


/**
 * @brief Generate list of supported databases
 * @return comma-separated list of databases
 */
std::vector<std::string> CGKeyFileAutoCDRipper::ReleaseDatabaseList()
{
    std::vector<std::string> rval;
    rval.reserve(NumberOfReleaseDatabases);

    for (int n = 0; n<NumberOfReleaseDatabases; n++)
        rval.push_back(to_string(static_cast<ReleaseDatabase>(n)));

    return rval;
}

///////////////////////////////////////////////////////////////////////////////////////////////

/** Create new key_file object with default values
 *
 * @brief Initialize an empty key file
 */
void CGKeyFileAutoCDRipper::Initialize_()
{
    string strval, comment;
    strval.reserve(1024);
    comment.reserve(1024);
    std::vector<string> strlist;

    g_key_file_set_comment (key_file, NULL, NULL, "AutoCDRipper Options", NULL);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // General Option Group

    CreateStringKey("General","DrivePath",
                    GeneralDrivePath(),
                    "CD drive path. An empty string indicates auto-detect.");

    CreateStringKey("General","FileFormat",
                    GenerateOutputFileFormatList(),
                    to_string(GeneralFileFormat()),
                    "Output file format. For now, only wavpack is supported.");

    CreateStringKey("General", "OutputDir",
                    GeneralOutputDir(),
                    "A base path to where the output files will be placed. Use \"$HOME\""
                    "for the user's home directory.");

    CreateStringKey("General","FileNamingScheme",
                    GeneralFileNamingScheme(),
                    "Output file naming convension. It uses a subset of the foobar2000/CUETools convension:\n"
                    "http://www.cuetools.net/wiki/Cuetools_templates\n"
                    "http://wiki.hydrogenaudio.org/index.php?title=Foobar2000:Titleformat_Reference\n"
                    "\n"
                    "Variables\n"
                    "   %artist% is replaced by the artist name.\n"
                    "   %album% is replaced by the album name.\n"
                    "   %year% is replaced by the album date.\n"
                    "   %genre% is replaced by the album genre.\n"
                    "   %discnumber% is replaced by the disc number in a multidisc release.\n"
                    "   %totaldiscs% is replaced by the total number of discs in a multidisc release.\n"
                    "   %cddbid% is replaced by the CDDB ID string.\n"
                    "   %totaltracks% is replaced by the number of tracks on the disc.\n"
                    "   %unique% is replaced by the number starting from 1, and increased until the output path is unique.\n"
                    "Conditional Stion\n"
                    "   Text encolosed in brackets is removed if variables inside it are undefined.\n"
                    "Quotation Mark\n"
                    "   Text enclosed in single quotation marks, e.g. '(%)' is inserted bypassing "
                    "syntax processing; allows special characters such as %,$,[,] to be inserted. "
                    "In order to insert a quotation mark character, use '' (two single quotation marks).");

    CreateBooleanKey("General","DoNotRipUnknownDisc",
                     GeneralDoNotRipUnknownDisc(),
                     "true to skip a CD if its information is not found in any of the databases.");

    CreateStringKey("General","FileNamingSchemeNoInfo",
                    GeneralFileNamingSchemeNoInfo(),
                    "If General::DoNoRipUnknownDisc=false, and the CD info is not available, "
                    "this file naming scheme is being used instead of General::FileNamingScheme. "
                    "Only %cddbid% %totaltracks% and %unique% variables may be used.");

    CreateBooleanKey("General","SkipTrackOnePreGap",
                     GeneralSkipTrackOnePreGap(),
                     "Setting this key to false keeps the pregap of the first track.");

    CreateStringListKey("General","DatabasePreferenceList",
                        ReleaseDatabaseList(),
                        GeneralDatabasePreferenceStringList(),
                        "Preference order of online CD/music database. "
                        "To populate the embedded cuesheet, the online "
                        "databases are queried/searched in the order given. "
                        "If a missing info from more preferred database is "
                        "found in the less preferred database, only the "
                        "missing information is used from the latter database. "
                        "A database omitted from this list will not be queried/"
                        "searched.");

    CreateBooleanKey("General","CoverArt",GeneralCoverArt(),
                     "Setting this key to true embeds CD artwork found online.");

    CreateIntegerKey("General","CoverArtPreferredSize",GeneralCoverArtPreferredSize(),
                     "Setting this key to true embeds CD artwork found online. If <=0 "
                     "largest size possible will be embedded.");

    CreateBooleanKey("General","ShowNotification",
                     GeneralShowNotification(),
                     "Setting this key to true to pop up desktop notification window ."
                     "indicating the ripping progress.");

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Rems Option Group: Sets which optional field to be stored in the REMS section
    // of the embedded cuesheet.

    CreateBooleanKey("Rems","DBINFO",RemsDbInfo(),
                     "true to embed source database and its disc/release ID.");

    CreateBooleanKey("Rems","DATE",RemsDate(),
                     "true to embed release date.");

    CreateBooleanKey("Rems","DATE",RemsLabel(),
                     "true to embed CD label name.");

    CreateBooleanKey("Rems","COUNTRY",RemsCountry(),
                     "true to embed country of origin.");

    CreateBooleanKey("Rems","UPC",RemsUPC(),
                     "true to embed barcode or universal product code.");

}
