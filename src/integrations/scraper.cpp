#include "integrations/scraper.hpp"
#include <iostream>
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

std::string scraper_fetch_url(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[Scraper] Failed to initialize curl." << std::endl;
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
        std::cerr << "[Scraper] curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    }

    curl_easy_cleanup(curl);
    return htmlBuffer;
}

std::string scraper_url_encode(const std::string& value) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return value;
    }
    char* encoded = curl_easy_escape(curl, value.c_str(), value.length());
    std::string result = value;
    if (encoded) {
        result = encoded;
        curl_free(encoded);
    }
    curl_easy_cleanup(curl);
    return result;
}

std::vector<ScrapedElement> scraper_xpath(
    const std::string& html, 
    const std::string& xpath_expr, 
    const std::vector<std::string>& attributes
) {
    std::vector<ScrapedElement> results;
    if (html.empty() || xpath_expr.empty()) {
        return results;
    }

    // Parse HTML with libxml2 HTML parser
    htmlDocPtr doc = htmlReadMemory(html.c_str(), html.length(), nullptr, nullptr, HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (!doc) {
        std::cerr << "[Scraper] Failed to parse HTML document." << std::endl;
        return results;
    }

    // Initialize XPath context
    xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
    if (!xpathCtx) {
        std::cerr << "[Scraper] Failed to create XPath context." << std::endl;
        xmlFreeDoc(doc);
        return results;
    }

    // Evaluate XPath expression
    xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(reinterpret_cast<const xmlChar*>(xpath_expr.c_str()), xpathCtx);
    if (!xpathObj) {
        std::cerr << "[Scraper] Failed to evaluate XPath expression: " << xpath_expr << std::endl;
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return results;
    }

    xmlNodeSetPtr nodes = xpathObj->nodesetval;
    if (nodes) {
        for (int i = 0; i < nodes->nodeNr; ++i) {
            xmlNodePtr node = nodes->nodeTab[i];
            ScrapedElement element;
            
            // Extract inner text content
            xmlChar* textProp = xmlNodeGetContent(node);
            if (textProp) {
                element.text = reinterpret_cast<char*>(textProp);
                xmlFree(textProp);
            }

            // Extract requested attributes
            for (const auto& attr : attributes) {
                xmlChar* attrVal = xmlGetProp(node, reinterpret_cast<const xmlChar*>(attr.c_str()));
                if (attrVal) {
                    element.attributes[attr] = reinterpret_cast<char*>(attrVal);
                    xmlFree(attrVal);
                }
            }

            results.push_back(element);
        }
    }

    // Cleanup resources
    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);

    return results;
}
