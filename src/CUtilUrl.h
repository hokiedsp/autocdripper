#pragma once

#include <string>
#include <atomic>
#include <mutex>
#include <vector>
#include <curl/curl.h>

typedef std::vector<unsigned char> UByteVector;

/** Abstract base Database class with libcurl object
 */
class CUtilUrl
{
public:
    /** Constructor.
     *
     *  @param[in] Client program name. If omitted or empty, uses "autorip"
     *  @param[in] Client program version. If omitted or empty, uses "alpha"
     */
    CUtilUrl(const std::string &cname="autorip",const std::string &cversion="alpha");

    /**
     * @brief Copy Constructor (creates duplicate curl session)
     * @param[in] source
     */
    CUtilUrl(const CUtilUrl &src);

    /** Destructor
     */
    virtual ~CUtilUrl();

    /** Get current value of static AutoCleanUpMode parameter
     *
     *  @return true if curl_global_cleanup is to be called automatically.
     */
    static bool GetAutoCleanUpMode(const bool mode) { return AutoCleanUp; }

    /** Set static AutoCleanUpMode parameter
     *
     *  @param[in] true if curl_global_cleanup is to be called automatically.
     */
    static void SetAutoCleanUpMode(const bool mode) { AutoCleanUp = mode; }

protected:
    CURL *curl; // pointer to the curl session
    std::string rawdata; // received data buffer

    /** Invoke this function to perform the HTTP transfer. Upon completion of the call,
     *  data member variable contains the received data. The previous content of data will
     *  be lost.
     *
     *  @brief Perform a blocking file transfer
     *  @param The URLof the other endpoint
     */
    virtual void PerformHttpTransfer_(const std::string &url);

    /**
     * @brief Get the length of remote content
     * @param[in] URL of the remote content
     * @return Length in bytes; if unknown, returns 0
     */
    virtual size_t GetHttpContentLength_(const std::string &url) const;

    /**
     * @brief Download the (binary) data from URL and store it in a memory block
     * @param[in] URL
     * @return unsigned char vector containing the downloaded data
     * @throw runtime_error if curl fails to retrieve the data
     * @throw length_error if requested data is too large
     */
    virtual UByteVector DataToMemory(const std::string &url);

    /** Default callback for writing received HTTP data
     *
     * @param[in]   Points to the delivered data
     * @param[in]   The actual size of that data in byte is size multiplied with nmemb
     * @param[in]   See above
     * @param[out]  std::string data buffer
     * @return      The number of bytes actually taken care of
     */
    static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);

    /** Callback for writing received HTTP data to std::vector<unsigned char> buffer
     *
     * @param[in]   Points to the delivered data
     * @param[in]   The actual size of that data in byte is size multiplied with nmemb
     * @param[in]   std::vector<unsigned char> buffer
     * @return      The number of bytes actually taken care of
     */
    static size_t write_uchar_vector_callback(char *ptr, size_t size, size_t nmemb, void *userdata);

private:
    static std::mutex globalmutex; /// mutex to make curl_global_init and curl_global_cleanup thread safe
    static int Nobjs; /// number of instantiated CUtilUrl objects
    static std::atomic_bool AutoCleanUp;    /// if true (default)

};
