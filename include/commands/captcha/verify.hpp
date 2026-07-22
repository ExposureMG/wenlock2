#pragma once
#include "../../handler.hpp"
#include "../../integrations/database.hpp"
#include <dpp/dpp.h>
#include <mutex>

inline Command create_captcha_verify_command(SQLite::Database& db, std::mutex& db_mutex) {
    return Command{
        .name = "verify",
        .description = "Check your CAPTCHA status and unlock server access",
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
                    "❌ Please use <#" + std::to_string(channel_id) + "> to verify."
                ).set_flags(dpp::m_ephemeral));
                return;
            }

            std::string user_id_str = std::to_string(event.command.usr.id);
            std::string url = "http://wenlock2-server:8080/check-status/" + user_id_str;

            // Capture role_id by value — it's read outside the lock after the HTTP callback
            event.owner->request(url, dpp::m_get, [event, role_id](const dpp::http_request_completion_t& res) {
                if (res.status != 200) {
                    event.reply(dpp::message(
                        "❌ Server communication failure."
                    ).set_flags(dpp::m_ephemeral));
                    return;
                }

                if (res.body == "verified") {
                    event.owner->guild_member_add_role(
                        event.command.guild_id,
                        event.command.usr.id,
                        role_id
                    );
                    event.reply(dpp::message(
                        "✅ Verification successful! Welcome to the server."
                    ).set_flags(dpp::m_ephemeral));
                } else if (res.body == "pending") {
                    event.reply(dpp::message(
                        "⚠️ You haven't completed the CAPTCHA yet. "
                        "Please finish it via the link from `/request`."
                    ).set_flags(dpp::m_ephemeral));
                } else {
                    event.reply(dpp::message(
                        "❌ No verification session found. Run `/request` first."
                    ).set_flags(dpp::m_ephemeral));
                }
            });
        }
    };
}