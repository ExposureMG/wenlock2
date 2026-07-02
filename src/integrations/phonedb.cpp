#include "integrations/phonedb.hpp"
#include "integrations/scraper.hpp"
#include <algorithm>

// Helper function to trim whitespace from a string
static std::string trim(const std::string& str) {
    if (str.empty()) return "";
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

std::vector<ScrapedTag> search_phonedb(const std::string& search_term) {
    std::string encoded = scraper_url_encode(search_term);
    std::string url = "https://phonedb.net/index.php?m=device&s=list&search_exp=" + encoded;
    std::string html = scraper_fetch_url(url);
    
    // Query all <a> tags that have a 'title' attribute
    auto elements = scraper_xpath(html, "//a[@title]", {"title", "href"});
    
    std::vector<ScrapedTag> results;
    results.reserve(elements.size());
    for (const auto& elem : elements) {
        ScrapedTag tag;
        tag.text = trim(elem.text);
        
        auto it_title = elem.attributes.find("title");
        if (it_title != elem.attributes.end()) {
            tag.title = trim(it_title->second);
        }
        
        auto it_href = elem.attributes.find("href");
        if (it_href != elem.attributes.end()) {
            tag.href = trim(it_href->second);
        }
        
        results.push_back(tag);
    }
    
    return results;
}

std::vector<std::pair<std::string, std::string>> get_phonedb_specs(const std::string& device_id) {
    std::string url = "https://phonedb.net/index.php?m=device&id=" + device_id;
    std::string html = scraper_fetch_url(url);
    
    // Select the <td> tags inside <tr> tags containing exactly 2 <td>s
    auto elements = scraper_xpath(html, "//tr[count(td)=2]/td", {});
    
    std::vector<std::pair<std::string, std::string>> specs;
    for (size_t i = 0; i + 1 < elements.size(); i += 2) {
        std::string key = trim(elements[i].text);
        std::string value = trim(elements[i+1].text);
        
        // Remove trailing colons from field names if present
        if (!key.empty() && key.back() == ':') {
            key.pop_back();
            key = trim(key);
        }
        
        if (!key.empty() || !value.empty()) {
            specs.push_back({key, value});
        }
    }
    
    return specs;
}
