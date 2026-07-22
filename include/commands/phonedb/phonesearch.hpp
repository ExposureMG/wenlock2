#pragma once
#include "handler.hpp"
#include "integrations/phonedb.hpp"
#include "integrations/scraper.hpp"
#include <sstream>

// PhoneDB pagination stride (devices per page)
static constexpr int PHONEDB_PAGE_STRIDE = 29;
// Maximum pages to fetch before giving up
static constexpr int PHONEDB_MAX_PAGES   = 3;

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

            // Acknowledge immediately — multiple fetches may take several seconds
            event.thinking();

            // URL-encode the query for use in embed URLs
            std::string encoded_query = scraper_url_encode(phone_query);
            auto query_tokens = phonedb_tokenise(phone_query);

            // Helper: filter raw scraped tags down to unique, valid device entries
            auto filter_devices = [](const std::vector<ScrapedTag>& raw,
                                     std::vector<ScrapedTag>& out) {
                for (const auto& tag : raw) {
                    if (tag.href.find("m=device") == std::string::npos) continue;
                    if (tag.href.find("id=")      == std::string::npos) continue;
                    if (tag.href.find("&d=")      != std::string::npos) continue;
                    if (tag.href.find("?d=")      != std::string::npos) continue;
                    if (tag.title.find("Add this item")        != std::string::npos) continue;
                    if (tag.title.find("detailed datasheet")   != std::string::npos) continue;
                    if (tag.title.empty()) continue;

                    // Dedup by href
                    bool dup = false;
                    for (const auto& d : out) {
                        if (d.href == tag.href) { dup = true; break; }
                    }
                    if (!dup) out.push_back(tag);
                }
            };

            // ── Paginated search loop ─────────────────────────────────────────
            // Fetch up to PHONEDB_MAX_PAGES pages.
            // Only advance to the next page when the current accumulated set
            // still contains zero valid device entries.
            std::vector<ScrapedTag> devices;

            for (int page = 0; page < PHONEDB_MAX_PAGES; ++page) {
                int offset = page * PHONEDB_PAGE_STRIDE;
                auto raw = search_phonedb(phone_query, offset);

                // If PhoneDB returned no anchor tags at all, we've gone past the
                // end of its results — no point fetching further pages.
                if (raw.empty()) break;

                filter_devices(raw, devices);

                // Stop as soon as we have at least one matching device
                if (!devices.empty()) break;
            }

            // ─────────────────────────────────────────────────────────────────

            if (devices.empty()) {
                dpp::embed embed = dpp::embed()
                    .set_color(dpp::colors::red)
                    .set_title("PhoneDB Search: " + phone_query)
                    .set_description("No Results")
                    .set_footer(
                        dpp::embed_footer()
                        .set_text("Wenlock")
                        .set_icon("https://raw.githubusercontent.com/GxOSS/.github/refs/heads/main/assets/gx_icon_colour_trans.png")
                    )
                    .set_timestamp(time(0));
                event.edit_original_response(dpp::message(event.command.channel_id, embed));
                return;
            }

            // Re-rank all collected devices by relevance to the query
            std::stable_sort(devices.begin(), devices.end(),
                [&query_tokens](const ScrapedTag& a, const ScrapedTag& b) {
                    return phonedb_relevance_score(a.title, query_tokens) >
                           phonedb_relevance_score(b.title, query_tokens);
                }
            );

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