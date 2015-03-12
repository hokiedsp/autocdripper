#pragma once

#include <string>
#include <glib.h>

class CGKeyFileBase
{
public:
    CGKeyFileBase(const std::string &filename);
    virtual ~CGKeyFileBase();

    virtual void Save();

protected:
    bool create_new;

    /** CGKeyFileBase will call derived class' Initialize_() during instantiation
     *  if its key file does not exist.
     *
     * @brief Initialize an empty key file
     */
    virtual void Initialize_()=0;

private:
    bool newfile;
    std::string file_name;
    GKeyFile *key_file;

};
