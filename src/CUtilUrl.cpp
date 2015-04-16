#include "CUtilUrl.h"

#include <stdexcept>
#include <iostream>

using std::string;
using std::mutex;
using std::runtime_error;

// initialize static member variables
int CUtilUrl::Nobjs = 0;
std::atomic_bool CUtilUrl::AutoCleanUp(true);
std::mutex CUtilUrl::globalmutex;

/** Constructor.
 *
 *  @param[in] Client program name. If omitted or empty, uses "autorip"
 *  @param[in] Client program version. If omitted or empty, uses "alpha"
 */
CUtilUrl::CUtilUrl(const std::string &cname,const std::string &cversion)
{
    // initialize curl object
    globalmutex.lock();
    curl = curl_easy_init();
    if (curl) Nobjs++;
    globalmutex.unlock();

    if (!curl) throw(runtime_error("Failed to start a libcurl session."));

    // reserve large enough data buffer
    rawdata.reserve(CURL_MAX_WRITE_SIZE);    // reserve memory for receive buffer

    // use static write_callback as the default write callback function
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CUtilUrl::write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rawdata);

    // set user agent
    curl_easy_setopt(curl, CURLOPT_USERAGENT, (cname+"/"+cversion).c_str());
}

/** Destructor
 */
CUtilUrl::~CUtilUrl()
{
    curl_easy_cleanup(curl);

    // decrement the number of instances and call global cleanup if this is the last object
    globalmutex.lock();
    Nobjs--;
    if (!Nobjs && AutoCleanUp) curl_global_cleanup();
    globalmutex.unlock();
}

/** Invoke this function to perform the HTTP transfer. Upon completion of the call,
 *  data member variable contains the received data. The previous content of data will
 *  be lost.
 *
 *  @brief Perform a blocking file transfer
 *  @param url
 */
void CUtilUrl::PerformHttpTransfer_(const std::string &url)
{
    // empty the buffer
    rawdata.clear();

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    CURLcode res = curl_easy_perform(curl);

    /* Check for errors */
    if(res != CURLE_OK) throw(std::runtime_error(curl_easy_strerror(res)));
}

/**
 * @brief Get the length of remote content
 * @param[in] URL of the remote content
 * @return Length in bytes; if unknown, returns 0
 */
size_t CUtilUrl::GetHttpContentLength_(const std::string &url) const
{
    double size;

    // set so only header is returned
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);

    // revert to receive the body
    curl_easy_setopt(curl, CURLOPT_NOBODY, 0);

    // if unknkown size (-1), return 0
    if (size<0.0) return 0;
    else return (size_t) size;
}

/* Default callback for writing received HTTP data
 *
 * @param[in]   Points to the delivered data
 * @param[in]   The actual size of that data in byte is size multiplied with nmemb
 * @param[in]   See above
 * @return      The number of bytes actually taken care of
 */
size_t CUtilUrl::write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size *= nmemb;
    ((string*)userdata)->append(ptr,size);
    return size;
}

size_t CUtilUrl::write_uchar_vector_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size *= nmemb;
    std::vector<unsigned char>* bindata = (std::vector<unsigned char>*)userdata;
    std::vector<unsigned char>::iterator it = bindata->end();
    bindata->insert(it, (unsigned char*)ptr,(unsigned char*)ptr+size);
    return size;
}
