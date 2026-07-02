#pragma once
#include <string>
#include <vector>

struct ScrapedTag {
    std::string title;
    std::string href;
    std::string text;
};

std::vector<ScrapedTag> search_phonedb(const std::string& search_term);