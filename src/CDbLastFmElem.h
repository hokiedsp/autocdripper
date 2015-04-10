#pragma once

#include "CUtilJson.h"

class CDbLastFmElem : protected CUtilJson
{
    friend class CDbLastFm;
public:
    CDbLastFmElem(const std::string &data);
    virtual ~CDbLastFmElem() {}

    /**
     * @brief Exchanges the content with another CDbLastFmElem object
     * @param Another CDbLastFmElem object
     */
    void Swap(CDbLastFmElem &other);

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
};
