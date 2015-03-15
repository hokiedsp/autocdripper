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
 * @brief Get General::SkipUnknownDisc option
 * @return true to skip disc if its information is not found in databases
 */
bool CGKeyFileAutoCDRipper::GeneralSkipUnknownDisc()
{
    GError *err = NULL;
    bool rval = g_key_file_get_boolean(key_file, "General", "SkipUnknownDisc", &err);
    if (err)
    {
        if ((err->code==G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                || (err->code==G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
            rval = true; // default value
        else
            throw(runtime_error("General::SkipUnknownDisc option value is invalid."));
    }
    return rval;
}

/**
 * @brief Get General::FileFormat option
 * @return Output file format as ReleaseDatabase enum type
 */
OutputFileFormat CGKeyFileAutoCDRipper::GeneralFileFormat()
{
    GError *err = NULL;
    char* keyval = g_key_file_get_string(key_file, "General", "FileFormat", &err);
    if (err)
    {
        if ((err->code==G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                || (err->code==G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
            return OutputFileFormat::WAVPACK; // if key value does not exist, use the default value
        else
            goto throw_except;
    }

    // Try converting the string value to OutputFileFormat value
    try
    {
        return sttooff(keyval);
    }
    catch (const std::invalid_argument& ia)
    {
        // proceed if failed to convert
    }

throw_except:
    throw(runtime_error("General::FileFormat option value is invalid."));

    return OutputFileFormat::WAVPACK; // should never get here
}

/**
 * @brief Get General::SkipTrackOnePreGap option
 * @return true to skip the first tracks' pregap
 */
bool CGKeyFileAutoCDRipper::GeneralSkipTrackOnePreGap()
{
    GError *err = NULL;
    bool rval = g_key_file_get_boolean(key_file, "General", "SkipTrackOnePreGap", &err);
    if (err)
    {
        if ((err->code==G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                || (err->code==G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
            rval = true; // default value
        else
            throw(runtime_error("General::SkipTrackOnePreGap option value is invalid."));
    }
    return rval;
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
    std::vector<ReleaseDatabase> rlist;

    GError *err = NULL;
    size_t len;
    char **strlist = g_key_file_get_string_list(key_file, "General", "DataBasePreferenceList", &len, &err);
    if (err)
    {
        if ((err->code==G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                || (err->code==G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
        {
            // if key value does not exist, use the default value
            rlist.reserve(4);
            rlist.emplace_back(ReleaseDatabase::DISCOGS);
            rlist.emplace_back(ReleaseDatabase::MUSICBRAINZ);
            rlist.emplace_back(ReleaseDatabase::FREEDB);
            rlist.emplace_back(ReleaseDatabase::LASTFM);

            return rlist;
        }
        else
        {
            goto throw_except;
        }
    }

    // Try converting the string value to OutputFileFormat value
    try
    {
        rlist.reserve(len);
        for (size_t n=0;n<len;n++)
            rlist.push_back(sttordb(strlist[n]));
        return rlist;
    }
    catch (const std::invalid_argument& ia)
    {
        // proceed if failed to convert
    }

throw_except:
    throw(runtime_error("General::DatabasePreferenceList option value is invalid."));

    return rlist; // should never get here
}

/**
 * @brief Get General::CoverArt option
 * @return true to embed cover art
 */
bool CGKeyFileAutoCDRipper::GeneralCoverArt()
{
    GError *err = NULL;
    bool rval = g_key_file_get_boolean(key_file, "General", "CoverArt", &err);
    if (err)
    {
        if ((err->code==G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                || (err->code==G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
            rval = true; // default value
        else
            throw(runtime_error("General::CoverArt option value is invalid."));
    }
    return rval;
}

/**
 * @brief Get General::CoverArtPreferredSize option
 * @return preferred dimension of the coverart image in pixels
 */
int CGKeyFileAutoCDRipper::GeneralCoverArtPreferredSize()
{
    GError *err = NULL;
    bool rval = g_key_file_get_boolean(key_file, "General", "CoverArtPreferredSize", &err);
    if (err)
    {
        if ((err->code==G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                || (err->code==G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
            rval = 300; // default value
        else
            throw(runtime_error("General::CoverArtPreferredSize option value is invalid."));
    }
    return rval;
}

/**
 * @brief Get General::ShowNotification option
 * @return true to enable notification
 */
int CGKeyFileAutoCDRipper::GeneralShowNotification()
{
    GError *err = NULL;
    bool rval = g_key_file_get_boolean(key_file, "General", "ShowNotification", &err);
    if (err)
    {
        if ((err->code==G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                || (err->code==G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
            rval = true; // default value
        else
            throw(runtime_error("General::ShowNotification option value is invalid."));
    }
    return rval;
}

/**
 * @brief Get Rems::DBINFO option
 * @return true to embed databse info (name and disc/release ID)
 */
bool CGKeyFileAutoCDRipper::RemsDbInfo()
{
    GError *err = NULL;
    bool rval = g_key_file_get_boolean(key_file, "Rems", "DBINFO", &err);
    if (err)
    {
        if ((err->code==G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                || (err->code==G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
            rval = true; // default value
        else
            throw(runtime_error("Rems::DBINFO option value is invalid."));
    }
    return rval;
}

/**
 * @brief Get Rems::DATE option
 * @return true to embed release date
 */
bool CGKeyFileAutoCDRipper::RemsDate()
{
    GError *err = NULL;
    bool rval = g_key_file_get_boolean(key_file, "Rems", "DATE", &err);
    if (err)
    {
        if ((err->code==G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                || (err->code==G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
            rval = true; // default value
        else
            throw(runtime_error("Rems::DATE option value is invalid."));
    }
    return rval;
}


/**
 * @brief Get Rems::LABEL option
 * @return true to embed cd label name
 */
bool CGKeyFileAutoCDRipper::RemsLabel()
{
    GError *err = NULL;
    bool rval = g_key_file_get_boolean(key_file, "Rems", "LABEL", &err);
    if (err)
    {
        if ((err->code==G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                || (err->code==G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
            rval = true; // default value
        else
            throw(runtime_error("Rems::LABEL option value is invalid."));
    }
    return rval;
}


/**
 * @brief Get Rems::COUNTRY option
 * @return true to embed country of origin
 */
bool CGKeyFileAutoCDRipper::RemsCountry()
{
    GError *err = NULL;
    bool rval = g_key_file_get_boolean(key_file, "Rems", "COUNTRY", &err);
    if (err)
    {
        if ((err->code==G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                || (err->code==G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
            rval = true; // default value
        else
            throw(runtime_error("Rems::COUNTRY option value is invalid."));
    }
    return rval;
}


/**
 * @brief Get Rems::UPC option
 * @return true to embed UPC barcode numbers
 */
bool CGKeyFileAutoCDRipper::RemsUPC()
{
    GError *err = NULL;
    bool rval = g_key_file_get_boolean(key_file, "Rems", "UPC", &err);
    if (err)
    {
        if ((err->code==G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                || (err->code==G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
            rval = true; // default value
        else
            throw(runtime_error("Rems::UPC option value is invalid."));
    }
    return rval;
}


/**
 * @brief Get Rems::ASIN option
 * @return true to embed Amazon product code
 */
bool CGKeyFileAutoCDRipper::RemsASIN()
{
    GError *err = NULL;
    bool rval = g_key_file_get_boolean(key_file, "Rems", "ASIN", &err);
    if (err)
    {
        if ((err->code==G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                || (err->code==G_KEY_FILE_ERROR_GROUP_NOT_FOUND))
            rval = true; // default value
        else
            throw(runtime_error("Rems::ASIN option value is invalid."));
    }
    return rval;
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

    CreateStringKey("General","FileFormat",
                    GenerateOutputFileFormatList(),
                    to_string(GeneralFileFormat()),
                    "Output file format. For now, only wavpack is supported.");

    CreateBooleanKey("General","SkipTrackOnePreGap",
                     GeneralSkipTrackOnePregap(),
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

    CreateBooleanKey("Rems","ASIN",RemsASIN(),
                     "true to embed Amazon.com product code.");

}
