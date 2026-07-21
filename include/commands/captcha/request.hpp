#pragma once
#include "../../handler.hpp"
#include <dpp/dpp.h>
#include <dpp/json.h>

inline Command create_captcha_request_command() {
    return Command{
        .name = "request",
        .description = "Get your personal CAPTCHA verification link",
        .parameters = {},
        .handler = [](const dpp::slashcommand_t& event) {
            std::string user_id = std::to_string(event.command.usr.id);
            std::string url = "http://webserver:8080/create-session/" + user_id;

            event.owner->request(url, dpp::m_post, [event](const dpp::http_request_completion_t& res) {
                if (res.status == 200) {
                    auto json_data = nlohmann::json::parse(res.body);
                    std::string captcha_url = json_data["url"];
                    
                    event.reply(dpp::message("🔗 Complete your CAPTCHA here: " + captcha_url + 
                                             "\nAfter finishing, type `/verify` to unlock access!")
                                .set_flags(dpp::m_ephemeral));
                } else {
                    event.reply(dpp::message("❌ Error connecting to server.").set_flags(dpp::m_ephemeral));
                }
            });
        }
    };
}

