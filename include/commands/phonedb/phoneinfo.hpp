#pragma once
#include "handler.hpp"
#include "integrations/phonedb.hpp"
#include <algorithm>

inline Command create_phone_info_command() {
    return Command{
        .name = "phoneinfo",
        .description = "Get the specs for a phone",
        .parameters = {
            {"name", {dpp::co_string, "The phone name to search for", false}},
            {"id", {dpp::co_string, "The PhoneDB device ID", false}}
        },
        .handler = [](const dpp::slashcommand_t& event) {
            std::string name_param;
            std::string id_param;

            try {
                name_param = std::get<std::string>(event.get_parameter("name"));
            } catch (...) {}

            try {
                id_param = std::get<std::string>(event.get_parameter("id"));
            } catch (...) {}

            if (name_param.empty() && id_param.empty()) {
                event.reply("Please specify either a phone name or a PhoneDB device ID.");
                return;
            }

            // Acknowledge the command since HTTP request/scraping takes time
            event.thinking();

            std::string final_id = id_param;
            std::string resolved_name = "";

            if (final_id.empty()) {
                // Search by name first to get the ID of the top result
                auto search_results = search_phonedb(name_param);
                std::vector<ScrapedTag> devices;
                
                for (const auto& tag : search_results) {
                    if (tag.href.find("m=device") != std::string::npos &&
                        tag.href.find("id=") != std::string::npos &&
                        tag.href.find("&d=") == std::string::npos &&
                        tag.href.find("?d=") == std::string::npos &&
                        tag.title.find("Add this item") == std::string::npos &&
                        tag.title.find("detailed datasheet") == std::string::npos &&
                        !tag.title.empty()) {
                        
                        devices.push_back(tag);
                    }
                }

                if (devices.empty()) {
                    dpp::embed embed = dpp::embed()
                        .set_color(dpp::colors::red)
                        .set_description("Could not find any phone matching: " + name_param);
                    event.edit_original_response(dpp::message(event.command.channel_id, embed));
                    return;
                }

                // Retrieve ID from the top result
                const auto& top_result = devices[0];
                resolved_name = top_result.title;

                size_t id_pos = top_result.href.find("id=");
                if (id_pos != std::string::npos) {
                    size_t start = id_pos + 3;
                    size_t end = top_result.href.find('&', start);
                    if (end == std::string::npos) {
                        final_id = top_result.href.substr(start);
                    } else {
                        final_id = top_result.href.substr(start, end - start);
                    }
                }
            }

            if (final_id.empty()) {
                dpp::embed embed = dpp::embed()
                    .set_color(dpp::colors::red)
                    .set_description("Failed to resolve device ID.");
                event.edit_original_response(dpp::message(event.command.channel_id, embed));
                return;
            }

            // Fetch specs for the ID
            auto specs = get_phonedb_specs(final_id);
            if (specs.empty()) {
                dpp::embed embed = dpp::embed()
                    .set_color(dpp::colors::red)
                    .set_description("Could not retrieve specifications for device ID `" + final_id + "`.");
                event.edit_original_response(dpp::message(event.command.channel_id, embed));
                return;
            }

            // Build dynamic embed title using Brand and Model if available
            std::string brand = "";
            std::string model = "";
            for (const auto& pair : specs) {
                if (pair.first == "Brand") brand = pair.second;
                if (pair.first == "Model") model = pair.second;
            }

            dpp::embed embed = dpp::embed()
                .set_color(dpp::colors::sti_blue)
                .set_url("https://phonedb.net/index.php?m=device&id=" + final_id)
                .set_footer(
                    dpp::embed_footer()
                    .set_text("PhoneDB ID: " + final_id)
                    .set_icon("https://raw.githubusercontent.com/GxOSS/.github/refs/heads/main/assets/gx_icon_colour_trans.png")
                )
                .set_timestamp(time(0));

            if (!brand.empty() || !model.empty()) {
                embed.set_title(brand + " " + model);
            } else if (!resolved_name.empty()) {
                embed.set_title(resolved_name);
            } else {
                embed.set_title("Device Specifications");
            }

            // Populate embed fields with the parsed specs
            size_t limit = std::min(specs.size(), size_t(24));
            for (size_t i = 0; i < limit; ++i) {
                std::string val = specs[i].second;
                if (val.length() > 1024) val = val.substr(0, 1021) + "...";
                embed.add_field(
                    specs[i].first,
                    val,
                    true // Display inline for compact grid view
                );
            }

            dpp::message msg(event.command.channel_id, embed);
            event.edit_original_response(msg);
        }
    };
}