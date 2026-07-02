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
                // Heuristic: Device link href contains 'm=device' and 'id=', but does not contain '&d=' (datasheet)
                if (tag.href.find("m=device") != std::string::npos &&
                    tag.href.find("id=") != std::string::npos &&
                    tag.href.find("&d=") == std::string::npos &&
                    tag.href.find("?d=") == std::string::npos &&
                    tag.title.find("Add this item") == std::string::npos &&
                    tag.title.find("detailed datasheet") == std::string::npos &&
                    !tag.title.empty()) {
                    
                    // Avoid duplicate entries if they appear
                    bool duplicate = false;
                    for (const auto& dev : devices) {
                        if (dev.href == tag.href) {
                            duplicate = true;
                            break;
                        }
                    }
                    if (!duplicate) {
                        devices.push_back(tag);
                    }
                }
            }

            dpp::embed embed = dpp::embed()
                .set_color(dpp::colors::sti_blue)
                .set_title("PhoneDB Search Results: " + phone_query)
                .set_url("https://phonedb.net/index.php?m=device&s=list&search_exp=" + encoded_query)
                .set_footer(
                    dpp::embed_footer()
                    .set_text("Wenlock")
                    .set_icon("https://raw.githubusercontent.com/GxOSS/.github/refs/heads/main/assets/gx_icon_colour_trans.png")
                )
                .set_timestamp(time(0));

            if (devices.empty()) {
                embed.set_description("No devices found matching your search query.");
            } else {
                embed.set_description("Here are the top results from PhoneDB:");
                // Limit to top 10 results to fit comfortably in a Discord embed
                size_t limit = std::min(devices.size(), size_t(10));
                for (size_t i = 0; i < limit; ++i) {
                    std::string device_url = "https://phonedb.net/" + devices[i].href;
                    std::string device_id = "";
                    size_t id_pos = devices[i].href.find("id=");
                    if (id_pos != std::string::npos) {
                        size_t start = id_pos + 3;
                        size_t end = devices[i].href.find('&', start);
                        if (end == std::string::npos) {
                            device_id = devices[i].href.substr(start);
                        } else {
                            device_id = devices[i].href.substr(start, end - start);
                        }
                    }

                    embed.add_field(
                        devices[i].title,
                        "**ID:** `" + device_id + "` | [View Specifications](" + device_url + ")",
                        false
                    );
                }
                if (devices.size() > 10) {
                    embed.add_field(
                        "More Results",
                        "[View all " + std::to_string(devices.size()) + " results on PhoneDB](" + 
                        "https://phonedb.net/index.php?m=device&s=list&search_exp=" + encoded_query + ")",
                        false
                    );
                }
            }

            dpp::message msg(event.command.channel_id, embed);
            event.edit_original_response(msg);
        }
    };
}