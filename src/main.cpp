#include "commands/ping.hpp"
#include "commands/embedtest.hpp"
#include "commands/phonedb/phonesearch.hpp"
#include "commands/phonedb/phoneinfo.hpp"
#include "commands/captcha/linkcaptcha.hpp"
#include "commands/captcha/request.hpp"
#include "commands/captcha/verify.hpp"
#include "handler.hpp"
#include "integrations/database.hpp"
#include "log.hpp"
#include <dotenv.hpp>
#include <dpp/dpp.h>
#include <mutex>

int main() {
  std::setvbuf(stdout, NULL, _IONBF, 0);
  std::setvbuf(stderr, NULL, _IONBF, 0);
  Log::Init();
  dotenv::init();

  // Initialise database
  SQLite::Database db = init_database();
  create_tables(db);
  std::mutex db_mutex;

  dpp::cluster bot(dotenv::getenv("TOKEN"));
  CommandManager cmd_manager;

  bot.on_log(dpp::utility::cout_logger());

  // Register all commands
  cmd_manager.add_command(create_ping_command());
  cmd_manager.add_command(create_embed_command());
  cmd_manager.add_command(create_phone_search_command());
  cmd_manager.add_command(create_phone_info_command());
  cmd_manager.add_command(create_linkcaptcha_command(db, db_mutex));
  cmd_manager.add_command(create_captcha_request_command(db, db_mutex));
  cmd_manager.add_command(create_captcha_verify_command(db, db_mutex));

  bot.on_ready([&bot, &cmd_manager](const dpp::ready_t &event) {
    if (dpp::run_once<struct register_bot_commands>()) {
      cmd_manager.register_commands(bot, dotenv::getenv("GUILD_ID"));
    }
  });

  bot.on_slashcommand([&cmd_manager](const dpp::slashcommand_t &event) {
    cmd_manager.handle_command(event);
  });

  bot.on_select_click([](const dpp::select_click_t &event) {
    if (event.custom_id == "test_dropdown") {
      std::string chosen = event.values.empty() ? "none" : event.values[0];
      event.reply(dpp::ir_update_message, "You selected option: " + chosen);
    } else if (event.custom_id == "phonesearch_result") {
      if (event.values.empty()) {
        event.reply(dpp::ir_update_message, "No device selected.");
        return;
      }

      const std::string device_id = event.values[0];

      // Acknowledge immediately — spec fetch may take a moment
      event.reply(dpp::ir_deferred_update_message, "");

      auto specs = get_phonedb_specs(device_id);
      if (specs.empty()) {
        dpp::embed err = dpp::embed()
            .set_color(dpp::colors::red)
            .set_description("Could not retrieve specifications for device ID `" + device_id + "`.");
        event.edit_original_response(dpp::message(err));
        return;
      }

      // Extract Brand and Model for the embed title
      std::string brand, model;
      for (const auto& pair : specs) {
        if (pair.first == "Brand") brand = pair.second;
        if (pair.first == "Model") model = pair.second;
      }

      dpp::embed embed = dpp::embed()
          .set_color(dpp::colors::sti_blue)
          .set_url("https://phonedb.net/index.php?m=device&id=" + device_id)
          .set_footer(
              dpp::embed_footer()
              .set_text("PhoneDB ID: " + device_id)
              .set_icon("https://raw.githubusercontent.com/GxOSS/.github/refs/heads/main/assets/gx_icon_colour_trans.png")
          )
          .set_timestamp(time(0));

      if (!brand.empty() || !model.empty()) {
        embed.set_title(brand + " " + model);
      } else {
        embed.set_title("Device Specifications");
      }

      size_t limit = std::min(specs.size(), size_t(24));
      for (size_t i = 0; i < limit; ++i) {
        std::string val = specs[i].second;
        if (val.length() > 1024) val = val.substr(0, 1021) + "...";
        embed.add_field(specs[i].first, val, true);
      }

      event.edit_original_response(dpp::message(embed));
    }
  });

  bot.start(dpp::st_wait);
  return 0;
}