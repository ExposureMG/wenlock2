#pragma once
#include "handler.hpp"
#include "integrations/phonedb.hpp"
#include "integrations/scraper.hpp"
#include <sstream>

inline Command create_phone_search_command() {
    return Command{
        .name = "phonesearch",
        .description = "Search PhoneDB for a phone",
        .parameters = {
            {"phone", {dpp::co_string, "The phone to search", true}}
        },
        .handler = [](const dpp::slashcommand_t& event) {
            std::string phone_query;
            try {
                phone_query = std::get<std::string>(event.get_parameter("phone"));
            } catch (...) {
                event.reply("Please provide a phone name to search for.");
                return;
            }

            if (phone_query.empty()) {
                event.reply("Search query cannot be empty.");
                return;
            }

            // Acknowledge the command first as scraping might take a few seconds
            event.thinking();

            // URL-encode the query for use in embed URLs
            std::string encoded_query = scraper_url_encode(phone_query);

            // Perform scraping
            std::vector<ScrapedTag> all_tags = search_phonedb(phone_query);
            std::vector<ScrapedTag> devices;

            // Filter for actual device link tags
            for (const auto& tag : all_tags) {
                if (tag.href.find("m=device") != std::string::npos &&
                    tag.href.find("id=") != std::string::npos &&
                    tag.href.find("&d=") == std::string::npos &&
                    tag.href.find("?d=") == std::string::npos &&
                    tag.title.find("Add this item") == std::string::npos &&
                    tag.title.find("detailed datasheet") == std::string::npos &&
                    !tag.title.empty()) {

                    // Avoid duplicate entries
                    bool duplicate = false;
                    for (const auto& dev : devices) {
                        if (dev.href == tag.href) { duplicate = true; break; }
                    }
                    if (!duplicate) devices.push_back(tag);
                }
            }

            // Re-rank results by relevance to the query
            auto query_tokens = phonedb_tokenise(phone_query);
            std::stable_sort(devices.begin(), devices.end(),
                [&query_tokens](const ScrapedTag& a, const ScrapedTag& b) {
                    return phonedb_relevance_score(a.title, query_tokens) >
                           phonedb_relevance_score(b.title, query_tokens);
                }
            );

            if (devices.empty()) {
                dpp::embed embed = dpp::embed()
                    .set_color(dpp::colors::red)
                    .set_title("PhoneDB Search: " + phone_query)
                    .set_description("No devices found matching your search query.")
                    .set_footer(
                        dpp::embed_footer()
                        .set_text("Wenlock")
                        .set_icon("https://raw.githubusercontent.com/GxOSS/.github/refs/heads/main/assets/gx_icon_colour_trans.png")
                    )
                    .set_timestamp(time(0));
                event.edit_original_response(dpp::message(event.command.channel_id, embed));
                return;
            }

            // Discord select menus allow at most 25 options
            size_t limit = std::min(devices.size(), size_t(25));

            dpp::component select_menu;
            select_menu.set_type(dpp::cot_selectmenu)
                       .set_id("phonesearch_result")
                       .set_placeholder("Select a device to view its specs");

            for (size_t i = 0; i < limit; ++i) {
                // Extract device ID from href
                std::string device_id;
                size_t id_pos = devices[i].href.find("id=");
                if (id_pos != std::string::npos) {
                    size_t start = id_pos + 3;
                    size_t end = devices[i].href.find('&', start);
                    device_id = (end == std::string::npos)
                        ? devices[i].href.substr(start)
                        : devices[i].href.substr(start, end - start);
                }

                if (device_id.empty()) continue;

                // Truncate label to Discord's 100-char limit
                std::string label = devices[i].title;
                if (label.length() > 100) label = label.substr(0, 97) + "...";

                select_menu.add_select_option(dpp::select_option(label, device_id));
            }

            dpp::embed embed = dpp::embed()
                .set_color(dpp::colors::sti_blue)
                .set_title("PhoneDB Search: " + phone_query)
                .set_description("Found **" + std::to_string(limit) + "** result" +
                    (limit == 1 ? "" : "s") +
                    ". Select a device below to view its specifications.")
                .set_url("https://phonedb.net/index.php?m=device&s=list&search_exp=" + encoded_query)
                .set_footer(
                    dpp::embed_footer()
                    .set_text("Wenlock")
                    .set_icon("https://raw.githubusercontent.com/GxOSS/.github/refs/heads/main/assets/gx_icon_colour_trans.png")
                )
                .set_timestamp(time(0));

            dpp::message msg(event.command.channel_id, embed);
            msg.add_component(
                dpp::component()
                    .set_type(dpp::cot_action_row)
                    .add_component(select_menu)
            );

            event.edit_original_response(msg);
        }
    };
}