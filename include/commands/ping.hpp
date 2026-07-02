#pragma once
#include "../handler.hpp"

inline Command create_ping_command() {
    return Command{
        .name = "ping",
        .description = "Pong!",
        .parameters = {
            {"message", {dpp::co_string, "Optional message to echo back", false}}
        },
        .handler = [](const dpp::slashcommand_t& event) {
            std::string msg = "Pong!";
            try {
                msg = "Pong: " + std::get<std::string>(event.get_parameter("message"));
            } catch (...) {
                // Parameter not provided
            }
            event.reply(msg);
        }
    };
}