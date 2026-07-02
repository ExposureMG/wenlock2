#include "handler.hpp"

void CommandManager::add_command(Command cmd) {
    commands.emplace(cmd.name, std::move(cmd));
}

void CommandManager::register_commands(dpp::cluster &bot, dpp::snowflake default_guild) {
    for (const auto &[name, cmd] : commands) {
        dpp::slashcommand sc(cmd.name, cmd.description, bot.me.id);

        for (const auto &[param_name, param] : cmd.parameters) {
            dpp::command_option opt(param.type, param_name, param.description);
            if (param.required) {
                opt.required = true;
            }
            sc.add_option(opt);
        }

        dpp::snowflake target_guild = cmd.guild_id != 0 ? cmd.guild_id : default_guild;
        if (target_guild != 0) {
            bot.guild_command_create(sc, target_guild);
        } else {
            bot.global_command_create(sc);
        }
    }
}

void CommandManager::handle_command(const dpp::slashcommand_t &event) {
    if (const auto it = commands.find(event.command.get_command_name()); it != commands.end()) {
        it->second.handler(event);
    }
}

void CommandManager::reply(const dpp::message &msg, const dpp::slashcommand_t &event) const {
    event.reply(msg);
}