#pragma once
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

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
