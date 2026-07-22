#include "integrations/phonedb.hpp"
#include "integrations/scraper.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

// Helper function to trim whitespace from a string
static std::string trim(const std::string& str) {
    if (str.empty()) return "";
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

std::vector<ScrapedTag> search_phonedb(const std::string& search_term, int filter_offset) {
    std::string encoded = scraper_url_encode(search_term);
    std::string url = "https://phonedb.net/index.php?m=device&s=list&search_exp=" + encoded;
    if (filter_offset > 0) {
        url += "&filter=" + std::to_string(filter_offset);
    }
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

/*
// Tokenise a string into lowercase words
inline std::vector<std::string> phonedb_tokenise(const std::string& s) {
    std::vector<std::string> tokens;
    std::istringstream ss(s);
    std::string word;
    while (ss >> word) {
        std::string lower;
        lower.reserve(word.size());
        for (char c : word) lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        tokens.push_back(lower);
    }
    return tokens;
}

// Score a device title against the query tokens. Higher score = better match.
inline int phonedb_relevance_score(const std::string& title, const std::vector<std::string>& query_tokens) {
    std::string lower_title;
    lower_title.reserve(title.size());
    for (char c : title) lower_title += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    int score = 0;
    int matched_tokens = 0;

    for (size_t i = 0; i < query_tokens.size(); ++i) {
        const auto& tok = query_tokens[i];
        size_t pos = lower_title.find(tok);
        if (pos != std::string::npos) {
            ++matched_tokens;
            // Bonus for appearing early in the title
            score += 10;
            score += static_cast<int>(20 - std::min<size_t>(pos, 20));

            // Extra bonus for consecutive token pairs appearing in order
            if (i + 1 < query_tokens.size()) {
                size_t next_pos = lower_title.find(query_tokens[i + 1], pos + tok.size());
                if (next_pos != std::string::npos) {
                    score += 15;
                }
            }
        }
    }
    // Strong bonus if all tokens matched
    if (matched_tokens == static_cast<int>(query_tokens.size())) {
        score += 30;
    }

    // Penalty: extra words in title beyond query hint at a less specific match
    auto title_tokens = phonedb_tokenise(title);
    int extra = static_cast<int>(title_tokens.size()) - matched_tokens;
    if (extra > 0) score -= extra;

    return score;
}
*/