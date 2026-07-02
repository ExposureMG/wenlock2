#pragma once
#include <dpp/dpp.h>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

struct CommandParameter {
    dpp::command_option_type type;
    std::string description;
    bool required;
};

struct Command {
    std::string name;
    std::string description;
    std::vector<std::pair<std::string, CommandParameter>> parameters;
    dpp::snowflake guild_id = 0;
    std::function<void(const dpp::slashcommand_t&)> handler;
};

class CommandManager {
public:
    void add_command(Command cmd);
    void register_commands(dpp::cluster &bot, dpp::snowflake default_guild = 0);
    void handle_command(const dpp::slashcommand_t &event);
    void reply(const dpp::message &msg, const dpp::slashcommand_t &event) const;

private:
    std::unordered_map<std::string, Command> commands;
};