#include <string>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <fstream>

#include <curl/curl.h>
#include <libxml/tree.h>

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

/**
 * print_element_names:
 * @a_node: the initial xml node to consider.
 *
 * Prints the names of the all the xml elements
 * that are siblings or children of a given xml node.
 */
void print_nodes(xmlNode *a_node, std::string indent)
{
    xmlNode *cur_node = NULL;
    //        void *	_private	: application data
    //        xmlElementType	type	: type number, must be second !
    //        const xmlChar *	name	: the name of the node, or the entity
    //        xmlChar *	content	: the content

    //        xmlNs *	ns	: pointer to the associated namespace
    //        struct _xmlAttr *	properties	: properties list
    //        xmlNs *	nsDef	: namespace definitions on this node
    //        void *	psvi	: for type/PSVI informations
    //        unsigned short	line	: line number
    //        unsigned short	extra	: extra data for XPath/XSLT

    //        struct _xmlNode *	children	: parent->childs link
    //        struct _xmlNode *	last	: last child link
    //        struct _xmlNode *	parent	: child->parent link
    //        struct _xmlNode *	next	: next sibling link
    //        struct _xmlNode *	prev	: previous sibling link
    //        struct _xmlDoc *	doc	: the containing document End of common p

    for (cur_node = a_node; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type==XML_ELEMENT_NODE)
        {
            // string node
            if (cur_node->children && (cur_node->children->type==XML_TEXT_NODE))
            {
                cout << indent << cur_node->name << "(s) " << cur_node->children->content << endl;
            }
            else
            {
                cout << indent << cur_node->name;

                // list node must have "list" in its name
                if (strstr((char*)cur_node->name,"list")) cout << "(l)";
                else cout << "(o)";

                if (cur_node->properties)
                {
                    _xmlAttr* attr = cur_node->properties;
                    //        xmlElementType	type	: XML_ATTRIBUTE_NODE, must be second !
                    //        const xmlChar *	name	: the name of the property

                    //        struct _xmlNode *	children	: the value of the property
                    //        struct _xmlNode *	last	: NULL
                    //        struct _xmlNode *	parent	: child->parent link
                    //        struct _xmlAttr *	next	: next sibling link
                    //        struct _xmlAttr *	prev	: previous sibling link
                    //        struct _xmlDoc *	doc	: the containing document

                    //        xmlNs *	ns	: pointer to the associated namespace
                    //        xmlAttributeType	atype	: the attribute type if validating
                    //        void *	psvi	: for type/PSVI informations

                    cout << "[" << attr->name;
                    if (attr->children) cout << "=" << attr->children->content;
                    attr = attr->next;
                    while (attr)
                    {
                        cout << ", " << attr->name;
                        if (attr->children) cout << "=""" << attr->children->content << """";
                        attr = attr->next;
                    }
                    cout << "]";
                }
                cout << endl;

                print_nodes(cur_node->children,indent+" ");
            }
        }
        else
        {
            xmlBufPtr buf;
            xmlBufNodeDump(buf,cur_node->doc,cur_node,indent.size(),0);
            cout << (char*)buf;
        }
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

        CURL *curl = curl_easy_init();
        CURLcode res;
        try
        {
            curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent.c_str());

            //            curl_easy_setopt(curl, CURLOPT_URL, "https://musicbrainz.org/ws/2/release/866de398-c244-4694-8a02-cb674a3fa2bb");
            curl_easy_setopt(curl, CURLOPT_URL, "https://musicbrainz.org/ws/2/release/866de398-c244-4694-8a02-cb674a3fa2bb?inc=labels+artists+recordings+artist-credits+url-rels");
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

        cout << data ;
    }
    catch (std::exception& e)
    {
        curl_global_cleanup();
        std::cerr << e.what() << std::endl;
        return 1;
    }

    /*
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     */
    LIBXML_TEST_VERSION

            xmlDocPtr doc; /* the resulting document tree */

    /*
    * The document being in memory, it have no base per RFC 2396,
    * and the "noname.xml" argument will serve as its base.
    */
    doc = xmlReadMemory(data.c_str(), data.size(), "", NULL, 0);

    if (doc == NULL)
    {
        fprintf(stderr, "Failed to parse document\n");
    }
    else
    {
        /*Get the root element node */
        xmlNodePtr root_element = xmlDocGetRootElement(doc);

        print_nodes(root_element,"");

        xmlFreeDoc(doc);
    }

    /*
     * Cleanup function for the XML library.
     */
    xmlCleanupParser();

    /*
     * this is to debug memory for regression tests
     */
    xmlMemoryDump();

    return 0;
}
