#pragma once

#include "CUtilXmlTree.h"

class CDbAmazonElem: protected CUtilXmlTree
{
    friend class CDbAmazon;
public:
    CDbAmazonElem(const std::string &asin, const std::string &data);
    virtual ~CDbAmazonElem() {}

    /**
     * @brief Load New Data. Clears existing data first. If empty, no action.
     * @param valid XML string
     * @throw runtime_error if invalid XML string is passed in.
     */
    virtual void LoadData(const std::string &asin, const std::string &rawdata);

    /** Return a unique release ID string
     *
     *  @return id string if Query was successful.
     */
    std::string ReleaseId() const;

    /**
     * @brief Returns true if release data has a link to coverart images
     * @return true if there is a link
     */
    bool HasImage() const;

    /**
     * @brief Look through JSON strutcure for the URL to the image with requested size
     * @param[in] 0-"small", 1-"medium", 2-"large", 3-"extralarge","mega"
     * @return URL string to the image
     */
    std::string ImageURL(int CoverArtSize) const;

private:
    std::string asin;
};
