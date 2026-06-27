#pragma once
#include <dpp/dpp.h>
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>

struct Command {
    std::string name;
    std::string description;
    std::function<void(const dpp::slashcommand_t&)> handler;
};

class CommandManager {
public:
    void add_command(const Command& cmd);
    void register_commands(dpp::cluster& bot, const std::string& guild_id);
    void handle_command(const dpp::slashcommand_t& event);

private:
    std::unordered_map<std::string, Command> commands;
};