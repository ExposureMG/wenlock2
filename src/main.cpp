#include <dpp/dpp.h>
#include <dotenv.hpp>
#include "handler.hpp"
#include "commands/ping.hpp"

int main() {
    dotenv::init();
    dpp::cluster bot(dotenv::getenv("TOKEN"));
    CommandManager cmd_manager;

    bot.on_log(dpp::utility::cout_logger());

    // Register all commands
    cmd_manager.add_command(create_ping_command());

    bot.on_ready([&bot, &cmd_manager](const dpp::ready_t & event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            cmd_manager.register_commands(bot, dotenv::getenv("GUILD_ID"));
        }
    });

    bot.on_slashcommand([&cmd_manager](const dpp::slashcommand_t & event) {
        cmd_manager.handle_command(event);
    });

    bot.start(dpp::st_wait);
    return 0;
}