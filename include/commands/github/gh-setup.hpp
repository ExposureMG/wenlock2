// Link github org or account to bot
#pragma once
#include "../handler.hpp"

inline Command create_ghsetup_command() {
    return Command{
        .name = "gh-setup",
        .description = "Connect to a github user or organisation",
        .handler = [](const dpp::slashcommand_t& event) {
            std::string msg = "Pong!";
            try {
                msg = "Pong";
            } catch (...) {
                // Parameter not provided
            }
            event.reply(msg);
        }
    };
}