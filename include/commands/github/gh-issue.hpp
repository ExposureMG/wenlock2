// Link github org or account to bot
#pragma once
#include "../handler.hpp"

inline Command create_ghsetup_command() {
    return Command{
        .name = "gh-issue",
        .description = "Create a github issue",
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