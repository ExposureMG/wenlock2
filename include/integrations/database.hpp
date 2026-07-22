#pragma once
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#include <dpp/dpp.h>
#include <string>
#include <utility>

//  Lifecycle 
SQLite::Database init_database();
void create_tables(SQLite::Database& db);

//  Guild / User init 
bool init_guild(dpp::snowflake guild_id, SQLite::Database& db);
bool init_user(dpp::snowflake user_id, SQLite::Database& db);
bool init_guild_user(dpp::snowflake guild_id, dpp::snowflake user_id, SQLite::Database& db);

void delete_guild(dpp::snowflake guild_id, SQLite::Database& db);
void delete_user(dpp::snowflake user_id, SQLite::Database& db);
void delete_guild_user(dpp::snowflake guild_id, dpp::snowflake user_id, SQLite::Database& db);

//  Properties 
void set_user_property(dpp::snowflake user_id, const std::string& key,
                       const std::string& value, SQLite::Database& db);
void set_guild_property(dpp::snowflake guild_id, const std::string& key,
                        const std::string& value, SQLite::Database& db);
void set_guild_user_property(dpp::snowflake guild_id, dpp::snowflake user_id,
                             const std::string& key, const std::string& value,
                             SQLite::Database& db);

//  Captcha config 
// Store / update a guild's captcha channel and verified role.
void set_captcha_config(dpp::snowflake guild_id, dpp::snowflake channel_id,
                        dpp::snowflake role_id, SQLite::Database& db);

// Returns {channel_id, role_id}. Both are 0 if no config exists for the guild.
std::pair<dpp::snowflake, dpp::snowflake>
get_captcha_config(dpp::snowflake guild_id, SQLite::Database& db);