#include "handler.hpp"
#include <iostream>

void CommandManager::add_command(const Command& cmd) {
    commands[cmd.name] = cmd;
}

void CommandManager::register_commands(dpp::cluster& bot, const std::string& guild_id) {
    std::vector<dpp::slashcommand> slash_commands;
    for (const auto& [name, cmd] : commands) {
        slash_commands.emplace_back(name, cmd.description, bot.me.id);
    }
    bot.guild_bulk_command_create(slash_commands, guild_id);
}

void CommandManager::handle_command(const dpp::slashcommand_t& event) {
    auto it = commands.find(event.command.get_command_name());
    if (it != commands.end()) {
        it->second.handler(event);
    }
}