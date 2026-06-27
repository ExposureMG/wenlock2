#pragma once
#include "../handler.hpp"

inline Command create_ping_command() {
    return Command{
        .name = "ping",
        .description = "Pong!",
        .handler = [](const dpp::slashcommand_t& event) {
            event.reply("Pong!");
        }
    };
}