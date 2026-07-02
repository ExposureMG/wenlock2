#pragma once
#include "handler.hpp"

inline Command create_phone_info_command() {
    return Command{
        .name = "phoneinfo",
        .description = "Get the specs for a phone",
        .parameters = {
            {"phone", {dpp::co_string, "The phone to search", false}}
        },
        .handler = [](const dpp::slashcommand_t& event) {
	        dpp::embed embed = dpp::embed()
	            .set_color(dpp::colors::sti_blue)
	            .set_title("PhoneDB")
	            .set_url("https://phonedb.net/")
	            // .set_author("Wenlock", "https://gxoss.github.io/", "https://raw.githubusercontent.com/GxOSS/.github/refs/heads/main/assets/gx_icon_colour_trans.png")
	            .add_field(
	                "Regular field title",
	                "Some value here"
                )
	            .add_field(
	                "Inline field title",
	                "Some value here",
	                true
                )
	            .add_field(
	                "Inline field title",
	                "Some value here",
	                true
                )
	            // .set_image("https://dpp.dev/DPP-Logo.png")
	            .set_footer(
	                dpp::embed_footer()
	                .set_text("Wenlock")
                    .set_icon("https://raw.githubusercontent.com/GxOSS/.github/refs/heads/main/assets/gx_icon_colour_trans.png")
	            )
	            .set_timestamp(time(0));
	 
	        /* Create a message with the content as our new embed. */
	        dpp::message msg(event.command.channel_id, embed);
            event.reply(msg);
        }
    };
}