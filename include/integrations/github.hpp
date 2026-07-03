#pragma once
#include <optional>
#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include <cctype>
#include <algorithm>

struct GithubIssue {
    std::string title;
    std::optional<std::string> body;
    std::optional<std::string> type;
    std::optional<std::vector<std::string>> asignees;
    std::optional<std::vector<std::string>> labels;
    std::optional<std::vector<std::string>> projects
}