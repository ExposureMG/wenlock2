#pragma once
#include <string>
#include <vector>
#include <map>

// Generic element structure parsed from HTML
struct ScrapedElement {
    std::string text;
    std::map<std::string, std::string> attributes;
};

// Fetches the raw HTML contents of a URL
std::string scraper_fetch_url(const std::string& url);

// URL encodes a string using libcurl
std::string scraper_url_encode(const std::string& value);

// Scrapes HTML using an XPath query and extracts the requested attributes + inner text
std::vector<ScrapedElement> scraper_xpath(
    const std::string& html, 
    const std::string& xpath_expr, 
    const std::vector<std::string>& attributes
);
