#include "handler.hpp"

void CommandManager::add_command(Command cmd) {
  commands.emplace(cmd.name, std::move(cmd));
}

void CommandManager::register_commands(dpp::cluster &bot,
                                       dpp::snowflake guild_id) {
  std::vector<dpp::slashcommand> slash_commands;
  slash_commands.reserve(commands.size());

  for (const auto &[name, cmd] : commands) {
    slash_commands.emplace_back(name, cmd.description, bot.me.id);
  }

  bot.guild_bulk_command_create(slash_commands, guild_id);
}

void CommandManager::handle_command(const dpp::slashcommand_t &event) {
  if (const auto it = commands.find(event.command.get_command_name());
      it != commands.end()) {
    it->second.handler(event);
  }
}