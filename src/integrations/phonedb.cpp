#include "integrations/phonedb.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>

// Callback function for libcurl to write received data into a string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::string* s = static_cast<std::string*>(userp);
    s->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// Function to fetch URL content using libcurl
static std::string fetchUrl(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize curl." << std::endl;
        return "";
    }

    std::string htmlBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &htmlBuffer);
    
    // Follow redirects and set user agent to mimic a real browser
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");
    
    // Disable SSL verification to prevent issues with untrusted certificate authorities
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    }

    curl_easy_cleanup(curl);
    return htmlBuffer;
}

// Function to parse HTML and extract <a> tags containing a "title" attribute
static std::vector<ScrapedTag> scrapeTags(const std::string& html) {
    std::vector<ScrapedTag> results;
    if (html.empty()) {
        return results;
    }

    // Parse HTML with libxml2 HTML parser
    htmlDocPtr doc = htmlReadMemory(html.c_str(), html.length(), nullptr, nullptr, HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (!doc) {
        std::cerr << "Failed to parse HTML document." << std::endl;
        return results;
    }

    // Initialize XPath context
    xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
    if (!xpathCtx) {
        std::cerr << "Failed to create XPath context." << std::endl;
        xmlFreeDoc(doc);
        return results;
    }

    // Find all <a> tags that have a "title" attribute
    xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(reinterpret_cast<const xmlChar*>("//a[@title]"), xpathCtx);
    if (!xpathObj) {
        std::cerr << "Failed to evaluate XPath expression." << std::endl;
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return results;
    }

    xmlNodeSetPtr nodes = xpathObj->nodesetval;
    if (nodes) {
        for (int i = 0; i < nodes->nodeNr; ++i) {
            xmlNodePtr node = nodes->nodeTab[i];
            
            ScrapedTag tag;
            
            // Extract the "title" attribute
            xmlChar* titleProp = xmlGetProp(node, reinterpret_cast<const xmlChar*>("title"));
            if (titleProp) {
                tag.title = reinterpret_cast<char*>(titleProp);
                xmlFree(titleProp);
            }

            // Extract the "href" attribute
            xmlChar* hrefProp = xmlGetProp(node, reinterpret_cast<const xmlChar*>("href"));
            if (hrefProp) {
                tag.href = reinterpret_cast<char*>(hrefProp);
                xmlFree(hrefProp);
            }

            // Extract inner text
            xmlChar* textProp = xmlNodeGetContent(node);
            if (textProp) {
                tag.text = reinterpret_cast<char*>(textProp);
                xmlFree(textProp);
            }

            results.push_back(tag);
        }
    }

    // Cleanup libxml2 resources
    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);

    return results;
}

std::vector<ScrapedTag> search_phonedb(const std::string& search_term) {
    // URL-encode the search term
    CURL* curl = curl_easy_init();
    char* encodedSearchTerm = nullptr;
    if (curl) {
        encodedSearchTerm = curl_easy_escape(curl, search_term.c_str(), search_term.length());
    }
    
    std::string query = encodedSearchTerm ? encodedSearchTerm : search_term;
    if (encodedSearchTerm) {
        curl_free(encodedSearchTerm);
    }
    if (curl) {
        curl_easy_cleanup(curl);
    }

    std::string url = "https://phonedb.net/index.php?m=device&s=list&search_exp=" + query;
    std::string html = fetchUrl(url);
    return scrapeTags(html);
}
