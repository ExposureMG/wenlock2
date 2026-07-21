#pragma once
#include "../../handler.hpp"
#include <dpp/dpp.h>

inline Command create_captcha_verify_command() {
    return Command{
        .name = "verify",
        .description = "Check status and get server access after completing CAPTCHA",
        .parameters = {},
        .handler = [](const dpp::slashcommand_t& event) {
            constexpr dpp::snowflake VERIFIED_ROLE_ID = 123456789012345678; // Change to your Role ID
            std::string user_id = std::to_string(event.command.usr.id);
            std::string url = "http://wenlock2-server:8080/check-status/" + user_id;

            event.owner->request(url, dpp::m_get, [event, user_id](const dpp::http_request_completion_t& res) {
                if (res.status == 200) {
                    if (res.body == "verified") {
                        // Grant user their role
                        event.owner->guild_member_add_role(event.command.guild_id, event.command.usr.id, VERIFIED_ROLE_ID);
                        event.reply(dpp::message("✅ Verification successful! Welcome to the server.").set_flags(dpp::m_ephemeral));
                    } 
                    else if (res.body == "pending") {
                        event.reply(dpp::message("⚠️ You haven't completed the CAPTCHA yet. Please finish it via the link provided by `/request`.").set_flags(dpp::m_ephemeral));
                    } 
                    else {
                        event.reply(dpp::message("❌ No verification session found. Run `/request` first.").set_flags(dpp::m_ephemeral));
                    }
                } else {
                    event.reply(dpp::message("❌ Server communication failure.").set_flags(dpp::m_ephemeral));
                }
            });
        }
    };
}