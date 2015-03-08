#include "CDbHttpBase.h"

#include <stdexcept>

using std::string;
using std::mutex;

// initialize static member variables
std::atomic_int CDbHttpBase::Nobjs = 0;
std::atomic_bool CDbHttpBase::AutoCleanUp = true;

/** Constructor.
 *
 *  @param[in] Client program name. If omitted or empty, uses "autorip"
 *  @param[in] Client program version. If omitted or empty, uses "alpha"
 */
CDbHttpBase::CDbHttpBase(const std::string &cname,const std::string &cversion)
{
    // initialize curl object
    globalmutex.lock();
    curl = curl_easy_init();
    if (curl) Nobjs++;
    globalmutex.unlock();

    if (!curl) throw(runtime_error("Failed to start a libcurl session."));

    // reserve large enough data buffer
    data.reserve(CURL_MAX_WRITE_SIZE);    // reserve memory for receive buffer

    // use static write_callback as the default write callback function
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CDbHttpBase::write_callback());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, *data);

    // set user agent
    url_easy_setopt(curl, CURLOPT_USERAGENT, (cname+"/"+cversion).c_str());
}

/** Destructor
 */
CDbHttpBase::~CDbHttpBase()
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
CDHttpBase::Perform(const std::string &url)
{
    // empty the buffer
    data.clear();

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    CURLcode res = curl_easy_perform(curl);

    /* Check for errors */
    if(res != CURLE_OK) throw(std::runtime_error(curl_easy_strerror(res)));
}

/* Default callback for writing received HTTP data
 *
 * @param[in]   Points to the delivered data
 * @param[in]   The actual size of that data in byte is size multiplied with nmemb
 * @param[in]   See above
 * @return      The number of bytes actually taken care of
 */
size_t CDbHttpBase::write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size* = nmemb;
    dynamic_dcast<string*>(userdata)->append(ptr,size);
    return size;
}
