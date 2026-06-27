#pragma once
#include <dpp/dpp.h>
#include <functional>
#include <string>
#include <unordered_map>

struct Command {
  std::string name;
  std::string description;
  std::function<void(const dpp::slashcommand_t &)> handler;
};

class CommandManager {
public:
  void add_command(Command cmd);
  void register_commands(dpp::cluster &bot, dpp::snowflake guild_id);
  void handle_command(const dpp::slashcommand_t &event);

private:
  std::unordered_map<std::string, Command> commands;
};