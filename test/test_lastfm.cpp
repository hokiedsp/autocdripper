#include <iostream>
#include <memory>
#include <string>

#include <cstdlib>
#include <curl/curl.h>
#include <jansson.h>

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::unique_ptr;

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

std::string data;   // buffer

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size*=nmemb;
    data.append(ptr,size);
    return size;
}

int main (void)
{
    CURL *curl;
    CURLcode res;

    std::string useragent("autocdripper/testing");
    const string baseurl = "http://ws.audioscrobbler.com/2.0/";
    const string APIKey = "0691e8527e395f789d23e4e91b0be8fc";
    const string Secret = "ce74c6a6955a7e9e2a2e6ed8cfa796a3";

    string mbid("879909d6-1d83-4cfa-b260-8064afe2a756");

    string url(baseurl+"?method=album.getinfo&api_key="+APIKey+"&mbid="+mbid+"&format=json");

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent.c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1); // verify the peer's certificate
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2); // server must be verified by the certificate
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    cout << url << endl;

    // receive server response ...
    curl_easy_setopt(curl, CURLOPT_URL,url.c_str());
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) throw(std::runtime_error(curl_easy_strerror(res)));

    cout << "Returned data:" << endl;
    cout << data << endl << endl;

    // now load the data onto json object
    json_error_t error;
    json_t *root = json_loadb(data.c_str(), data.size(), 0, &error);

    if(!root)
    {
        std::cerr << "error: on line " << error.line << ": " << error.text << std::endl;
        return 1;
    }

    /* obj is a JSON object */
    const char *key;
    json_t *value;
    json_object_foreach(root, key, value)
    {
        json_print(std::cout, key,value,"");
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return 0;
}
