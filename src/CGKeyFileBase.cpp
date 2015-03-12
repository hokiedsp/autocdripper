#include "CGKeyFileBase.h"

#include <stdexcept>

using std::string;
using std::runtime_error;

CGKeyFileBase::CGKeyFileBase(const std::string &filename)
    : create_new(true), file_name(filename)
{
    GError  *error;

    // create a new CONF file
    key_file = g_key_file_new();

    // try to open the file
    if (!g_key_file_load_from_file(key_file,filename.c_str(),G_KEY_FILE_KEEP_COMMENTS,&error))
    {
        // Throw an exception to a couple critical types of error
        switch (*error)
        {
        case G_KEY_FILE_ERROR_UNKNOWN:
            throw(runtime_error("Failed to load CONF file: Encoding the text being parsed was in an unknown encoding."));
        case G_KEY_FILE_ERROR_PARSE:
            throw(runtime_error("Failed to load CONF file: CONF file was ill-formed."));
        case G_KEY_FILE_ERROR_NOT_FOUND:
            // Key file needs to be created. Initialize first
            Initialize_();
            newfile = true; // mark it new
            break;
        default:
            throw(runtime_error("Failed to load CONF file."));
        }

    }

}

CGKeyFileBase::~CGKeyFileBase()
{
    g_key_file_free(key_file);
}

void CGKeyFileBase::Save()
{
    // if new file and derived class
    if (newfile && !create_new) return;

    if (!g_key_file_save_to_file(key_file, filename, NULL))
        throw(runtime_error("Failed to save CONF file."));
}


//G_KEY_FILE_ERROR_KEY_NOT_FOUND //a requested key was not found
//G_KEY_FILE_ERROR_GROUP_NOT_FOUND //a requested group was not found
//G_KEY_FILE_ERROR_INVALID_VALUE //a value could not be parsed
