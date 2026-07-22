#pragma once
#include "../../handler.hpp"
#include "../../integrations/database.hpp"
#include <dpp/dpp.h>
#include <dpp/json.h>
#include <mutex>

inline Command create_captcha_request_command(SQLite::Database& db, std::mutex& db_mutex) {
    return Command{
        .name = "request",
        .description = "Get your personal CAPTCHA verification link",
        .parameters = {},
        .handler = [&db, &db_mutex](const dpp::slashcommand_t& event) {
            // Fetch config under lock — quick read, released immediately
            dpp::snowflake channel_id, role_id;
            {
                std::lock_guard<std::mutex> lock(db_mutex);
                auto [ch, ro] = get_captcha_config(event.command.guild_id, db);
                channel_id = ch;
                role_id    = ro;
            }

            if (channel_id == 0) {
                event.reply(dpp::message(
                    "❌ Captcha hasn't been configured yet. "
                    "An admin must run `/linkcaptcha` first."
                ).set_flags(dpp::m_ephemeral));
                return;
            }

            if (event.command.channel_id != channel_id) {
                event.reply(dpp::message(
                    "❌ Please use <#" + std::to_string(channel_id) +
                    "> to request a verification link."
                ).set_flags(dpp::m_ephemeral));
                return;
            }

            std::string user_id = std::to_string(event.command.usr.id);
            std::string url = "http://wenlock2-server:8080/create-session/" + user_id;

            event.owner->request(url, dpp::m_post, [event](const dpp::http_request_completion_t& res) {
                if (res.status == 200) {
                    auto json_data = nlohmann::json::parse(res.body);
                    std::string captcha_url = json_data["url"];
                    event.reply(dpp::message(
                        "🔗 Complete your CAPTCHA here: " + captcha_url +
                        "\nAfter finishing, type `/verify` to unlock access!"
                    ).set_flags(dpp::m_ephemeral));
                } else {
                    event.reply(dpp::message(
                        "❌ Error connecting to server."
                    ).set_flags(dpp::m_ephemeral));
                }
            });
        }
    };
}
