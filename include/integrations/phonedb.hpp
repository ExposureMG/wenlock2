#pragma once
#include <string>
#include <vector>
#include <utility>

struct ScrapedTag {
    std::string title;
    std::string href;
    std::string text;
};

std::vector<ScrapedTag> search_phonedb(const std::string& search_term);

// Scrapes the details of a specific phone by ID
std::vector<std::pair<std::string, std::string>> get_phonedb_specs(const std::string& device_id);