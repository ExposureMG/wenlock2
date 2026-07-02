#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>

// Custom deleters for libxml2 smart pointers to ensure RAII compliance
struct XmlDocDeleter { void operator()(htmlDocPtr doc) { xmlFreeDoc(doc); } };
struct XmlXPathCtxDeleter { void operator()(xmlXPathContextPtr ctx) { xmlXPathFreeContext(ctx); } };
struct XmlXPathObjDeleter { void operator()(xmlXPathObjectPtr obj) { xmlXPathFreeObject(obj); } };

// libcurl callback to append network data directly into a C++ std::string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// Function to handle the hidden PHP form POST request
std::string fetchPhoneDBData(const std::string& postFields) {
    CURL* curl = curl_easy_init();
    if (!curl) return "";

    std::string responseBuffer;
    
    // Target PhoneDB's primary PHP script routing the search
    curl_easy_setopt(curl, CURLOPT_URL, "https://phonedb.net");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
    
    // Mimic a legitimate web browser header profile
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    
    // Hook up the C++ string buffer string
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);

    // Execute network request
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        return "";
    }
    return responseBuffer;
}

// Function to parse the returned HTML payload using libxml2 and XPath
void parseHTMLResponse(const std::string& htmlContent) {
    if (htmlContent.empty()) return;

    // Parse messy, real-world web HTML structurally without throwing errors on unclosed tags
    std::unique_ptr<xmlDoc, XmlDocDeleter> doc(
        htmlReadMemory(htmlContent.c_str(), htmlContent.length(), nullptr, nullptr, HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING)
    );
    if (!doc) return;

    std::unique_ptr<xmlXPathContext, XmlXPathCtxDeleter> xpathCtx(xmlXPathNewContext(doc.get()));
    if (!xpathCtx) return;

    // XPath targeting specific content (Update expression based on PhoneDB markup layout)
    const xmlChar* xpathExpr = (const xmlChar*)"//h2[@class='title']/a";
    std::unique_ptr<xmlXPathObject, XmlXPathObjDeleter> xpathObj(xmlXPathEvalExpression(xpathExpr, xpathCtx.get()));

    if (xpathObj && xpathObj->nodesetval) {
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        for (int i = 0; i < nodes->nodeNr; ++i) {
            xmlNodePtr node = nodes->nodeTab[i];
            
            // Extract text nodes safely
            std::unique_ptr<xmlChar, void(*)(void*)> content(xmlNodeGetContent(node), xmlFree);
            if (content) {
                std::cout << "Parsed Item: " << reinterpret_cast<char*>(content.get()) << std::endl;
            }
        }
    }
}