#pragma once
#include "../../handler.hpp"
#include "../../integrations/database.hpp"
#include <dpp/dpp.h>
#include <mutex>

inline Command create_linkcaptcha_command(SQLite::Database& db, std::mutex& db_mutex) {
    return Command{
        .name = "linkcaptcha",
        .description = "Set the captcha verification channel and role for this server",
        .parameters = {
            {"channel", {dpp::co_channel, "The channel to restrict /request and /verify to", true}},
            {"role",    {dpp::co_role,    "The role to grant users on successful verification", true}}
        },
        .handler = [&db, &db_mutex](const dpp::slashcommand_t& event) {
            // Require Manage Server permission (resolved perms include channel overwrites + admin flag)
            if (!event.command.get_resolved_permission(event.command.usr.id).can(dpp::p_manage_guild)) {
                event.reply(dpp::message(
                    "❌ You need the **Manage Server** permission to use this command."
                ).set_flags(dpp::m_ephemeral));
                return;
            }

            auto channel_id = std::get<dpp::snowflake>(event.get_parameter("channel"));
            auto role_id    = std::get<dpp::snowflake>(event.get_parameter("role"));

            {
                std::lock_guard<std::mutex> lock(db_mutex);
                set_captcha_config(event.command.guild_id, channel_id, role_id, db);
            }

            event.reply(dpp::message(
                "✅ Captcha channel set to <#" + std::to_string(channel_id) + ">. "
                "Verified users will receive <@&" + std::to_string(role_id) + ">."
            ).set_flags(dpp::m_ephemeral));
        }
    };
}
