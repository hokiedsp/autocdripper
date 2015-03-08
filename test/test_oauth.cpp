#include <iostream>
#include <memory>
#include <string>

#include <oauth.h>
#include <cstdlib>
#include <curl/curl.h>

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::unique_ptr;

std::string data;

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size*=nmemb;
    data.append(ptr,size);
    return size;
}

//int parse_reply(const char *reply, char **token, char **secret)
//{
//    int rc;
//    int ok=1;
//    char **rv = NULL;
//    rc = oauth_split_url_parameters(reply, &rv);
//    qsort(rv, rc, sizeof(char *), oauth_cmpstringp);
//    if( rc==2
//            && !strncmp(rv[0],"oauth_token=",11)
//            && !strncmp(rv[1],"oauth_token_secret=",18) ) {
//        ok=0;
//        if (token) *token =strdup(&(rv[0][12]));
//        if (secret) *secret=strdup(&(rv[1][19]));
//        printf("key: '%s'\nsecret: '%s'\n",*token, *secret); // XXX token&secret may be NULL.
//    }
//    if(rv) free(rv);
//    return ok;
//}

int main (void)
{
    CURL *curl;
    CURLcode res;

    std::string useragent("autocdripper/testing");
    const char* request_token_uri = "https://api.discogs.com/oauth/request_token";
    const char* c_key = "NOtdOVETNQeLRIfFwRRT";
    const char *c_secret = "KqguFvnNCQvEhrCqPnLnWcmXdQwGuYQo";

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent.c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1); // verify the peer's certificate
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2); // server must be verified by the certificate
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    // receive server response ...
    unique_ptr<char, decltype(free)*> req_url(
            oauth_sign_url2(request_token_uri, NULL, OA_PLAINTEXT, NULL,
                              c_key, c_secret, NULL, NULL),free);
    curl_easy_setopt(curl, CURLOPT_URL, req_url.get());
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) throw(std::runtime_error(curl_easy_strerror(res)));

    // Parse the token and secret token
    string token_secret;
    string token;
    size_t p0 = 0, p1, p2;
    while (p0<data.size() && (p1 = data.find("=",p0+1))!=string::npos)
    {
        p2 = data.find("&",p1+1);
        if (p2==string::npos) p2 = data.size(); // last parameter
        if (data.compare(p0,p1-p0,"oauth_token_secret")==0)
            token_secret = data.substr(p1+1,p2-p1-1);
        else if (data.compare(p0,p1-p0,"oauth_token")==0)
            token = data.substr(p1+1,p2-p1-1);
        else
            cerr << "Unknown parameter returned: " << data.substr(p0,p1-p0) << endl;

        // advance the starting position
        p0 = p2+1;
    }

    cout << " URL: " << "http://discogs.com/oauth/authorize?oauth_token="+token << endl;

    curl_easy_setopt(curl, CURLOPT_URL, ("http://discogs.com/oauth/authorize?oauth_token="+token).c_str());
    data.clear();
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) throw(std::runtime_error(curl_easy_strerror(res)));


//    req_url.reset(oauth_sign_url2("https://api.discogs.com/oauth/access_token", NULL, OA_PLAINTEXT, NULL,
//                              c_key, c_secret, token.c_str(), token_secret.c_str()));
//    curl_easy_setopt(curl, CURLOPT_URL, req_url.get());

//    /* Perform the request, res will get the return code */
//    data.clear();
//    res = curl_easy_perform(curl);
//    if(res != CURLE_OK) throw(std::runtime_error(curl_easy_strerror(res)));

    cout << "Returned data:" << endl;
    cout << data << endl << endl;


    /* always cleanup */
    curl_easy_cleanup(curl);
    curl_global_cleanup();

//    reply = oauth_http_get(req_url.c_str(),postarg);

//    if (!reply)
//    {
//        cerr << "HTTP request for an oauth request-token failed." << endl;
//        return 1;
//    }

//    if (parse_reply(reply, &t_key, &t_secret))
//    {
//        cerr << "did not receive a request-token." << endl;
//        return 2;
//    }

//    // The Request Token provided above is already authorized, for this test server
//    // so we may use it to request an Access Token right away.

//    cout << "Access token.." << endl;

//    req_url = oauth_sign_url2(access_token_uri, NULL, OA_HMAC, NULL, c_key, c_secret, t_key, t_secret);
//    reply = oauth_http_get(req_url,postarg);

//    if (!reply)
//    {
//        cerr << "HTTP request for an oauth access-token failed." << endl;
//        return 3;
//    }

//    if (parse_reply(reply, &t_key, &t_secret))
//    {
//        cerr << "did not receive an access-token." << endl;
//        return 4;
//    }

//    cout << "make some request.." << endl;
//    req_url = oauth_sign_url2(test_call_uri, NULL, OA_HMAC, NULL, c_key, c_secret, t_key, t_secret);
//    reply = oauth_http_get(req_url,postarg);

//    printf("query:'%s'\n",req_url);
//    printf("reply:'%s'\n",reply);
//    if(req_url) free(req_url);
//    if(postarg) free(postarg);
//    if (strcmp(reply,"bar=baz&method=foo+bar")) return (5);
//    if(reply) free(reply);
//    if(t_key) free(t_key);
//    if(t_secret) free(t_secret);
    return 0;
}
