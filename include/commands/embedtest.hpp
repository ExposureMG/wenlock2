#pragma once
#include "../handler.hpp"

inline Command create_embed_command() {
    return Command{
        .name = "embedtest",
        .description = "Test wenlock embeds",
        .parameters = {
            {"message", {dpp::co_string, "Optional message to echo back", false}}
        },
        .handler = [](const dpp::slashcommand_t& event) {
            event.reply(
            dpp::message("Select an option:")
                .add_component(
                    dpp::component()
                        .set_type(dpp::cot_action_row)
                        .add_component(
                            dpp::component()
                                .set_type(dpp::cot_selectmenu)
                            .set_id("test_dropdown")
                                .set_placeholder("Pick something")
                                .add_select_option(dpp::select_option("Option 1", "opt1"))
                                .add_select_option(dpp::select_option("Option 2", "opt2"))
                                .add_select_option(dpp::select_option("Option 3", "opt3"))
                        )
                )
            );
        }
    };
}

