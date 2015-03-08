#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>

#include <curl/curl.h>
#include <jansson.h>

using std::cout;
using std::cerr;
using std::endl;

std::string data;

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size*=nmemb;
    data.append(ptr,size);
    return size;
}

void json_print(std::ostream &os, const char *key, json_t *value, std::string pre);

void json_print_value(std::ostream &os, json_t *value,std::string pre)
{
    switch (json_typeof(value))
    {
    case JSON_OBJECT:
        const char *obj_key;
        json_t *obj_val;
        os << endl;
        json_object_foreach(value, obj_key, obj_val)
        {
            json_print(os, obj_key,obj_val,pre+" ");
        }
        break;
    case JSON_STRING:
        os << ": " << json_string_value(value) << endl;
        break;
    case JSON_INTEGER:
        os << ": " << json_integer_value(value) << endl;
        break;
    case JSON_REAL:
        os << ": " << json_real_value(value) << endl;
        break;
    case JSON_TRUE:
        os << ": " << "TRUE" << endl;
        break;
    case JSON_FALSE:
        os << ": " << "FALSE" << endl;
        break;
    case JSON_NULL:
        os << ": " << "NULL" << endl;
        break;
    default:
        os << ": " << "UNKNOWN" << endl;
    }
}

void json_print(std::ostream &os, const char *key, json_t *value, std::string pre)
{
    /* block of code that uses key and value */
    if (json_typeof(value)==JSON_ARRAY)
    {
        size_t index;
        json_t *array_val;
        json_array_foreach(value,index,array_val)
        {
            os << pre << key << "[" << index << "]";
            json_print_value(os, array_val,pre);
        }
    }
    else
    {
        os << pre << key;
        json_print_value(os, value,pre);
    }
}

int main(void)
{

//    Consumer Key 	NOtdOVETNQeLRIfFwRRT
//    Consumer Secret 	KqguFvnNCQvEhrCqPnLnWcmXdQwGuYQo
//    Request Token URL 	http://api.discogs.com/oauth/request_token
//    Authorize URL 	http://www.discogs.com/oauth/authorize
//    Access Token URL 	http://api.discogs.com/oauth/access_token

    curl_global_init(CURL_GLOBAL_DEFAULT);
    try
    {
        data.reserve(1024*1024);    // reserve memory for receive buffer

        std::string useragent("autocdripper/testing");

        CURL *curl;
        CURLcode res;

        curl_slist *headers = new curl_slist;

        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        headers = curl_slist_append(headers, "Authorization:");
        headers = curl_slist_append(headers, "        OAuth oauth_consumer_key=""NOtdOVETNQeLRIfFwRRT"",");
        headers = curl_slist_append(headers, "        oauth_nonce=""random_string_or_timestamp"",");
        headers = curl_slist_append(headers, "        oauth_signature=""KqguFvnNCQvEhrCqPnLnWcmXdQwGuYQo"",");
        headers = curl_slist_append(headers, "        oauth_signature_method=""PLAINTEXT"",");
        headers = curl_slist_append(headers, "        oauth_timestamp="current_timestamp",");
        headers = curl_slist_append(headers, "        oauth_callback="your_callback");





                User-Agent: some_user_agent

        curl = curl_easy_init();
        try
        {
            curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent.c_str());
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1); // verify the peer's certificate
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2); // server must be verified by the certificate

            curl_easy_setopt(curl, CURLOPT_URL, "https://api.discogs.com/oauth/request_token");

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            curl_setopt($ch, CURLOPT_URL,"http://xxxxxxxx.xxx/xx/xx");
            curl_setopt($ch, CURLOPT_POST, 1);
            curl_setopt($ch, CURLOPT_POSTFIELDS,
                        "dispnumber=567567567&extension=6");
            curl_setopt($ch, CURLOPT_HTTPHEADER, array('Content-Type: application/x-www-form-urlencoded'));


            // receive server response ...
            curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);


            curl_easy_setopt(curl, CURLOPT_URL, "https://api.discogs.com/releases/4775774");
            //curl_easy_setopt(curl, CURLOPT_URL, "https://api.discogs.com/releases/2449413");
            //curl_easy_setopt(curl, CURLOPT_URL, "https://api.discogs.com/masters/306556"); // master release
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

            /* Perform the request, res will get the return code */
            res = curl_easy_perform(curl);

            /* Check for errors */
            if(res != CURLE_OK) throw(std::runtime_error(curl_easy_strerror(res)));

            /* always cleanup */
            curl_easy_cleanup(curl);
        }
        catch (...)
        {
            curl_easy_cleanup(curl);
            throw;
        }
    }
    catch (std::exception& e)
    {
        curl_global_cleanup();
        std::cerr << e.what() << std::endl;
        return 1;
    }

    cout << data << endl;

    // now load the data onto json object
    json_error_t error;
    json_t *root = json_loadb(data.c_str(), data.size(), 0, &error);

    if(!root)
    {
        std::cerr << "error: on line " << error.line << ": " << error.text << std::endl;
        return 1;
    }

    std::ofstream file("discog.txt");

    /* obj is a JSON object */
    const char *key;
    json_t *value;
    json_object_foreach(root, key, value)
    {
        json_print(file, key,value,"");
    }

    return 0;
}
