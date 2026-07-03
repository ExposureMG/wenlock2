#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#include <dpp/dpp.h>
#include <dotenv.hpp>
#include <filesystem>
#include <stdexcept>

// Helper: Insert if not exists (returns true if inserted)
bool try_insert(SQLite::Database& db, const std::string& table,
                const std::string& key_col, int64_t key) {
    SQLite::Statement check(db, "SELECT 1 FROM " + table + " WHERE " + key_col + " = ?");
    check.bind(1, key);
    if (check.executeStep()) return false;

    SQLite::Statement insert(db, "INSERT INTO " + table + " (" + key_col + ") VALUES (?)");
    insert.bind(1, key);
    insert.exec();
    return true;
}

SQLite::Database init_database() {
    std::string db_name = dotenv::getenv("CLIENT_ID");
    if (db_name.empty()) {
        throw std::runtime_error("CLIENT_ID environment variable not set");
    }
    std::filesystem::create_directories("db");
    return SQLite::Database("db/" + db_name,
                           SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
}

void create_tables(SQLite::Database& db) {
    db.exec("CREATE TABLE IF NOT EXISTS guild_settings ("
            "guild_id INTEGER PRIMARY KEY, "
            "prefix TEXT DEFAULT '!', "
            "log_channel INTEGER, welcome_channel INTEGER, "
            "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)");

    db.exec("CREATE TABLE IF NOT EXISTS user_settings ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "setting_key TEXT NOT NULL, setting_value TEXT, "
            "UNIQUE(setting_key))");
}

bool init_guild(dpp::snowflake guild_id, SQLite::Database& db) {
    return try_insert(db, "guild_settings", "guild_id", static_cast<int64_t>(guild_id));
}

bool init_user(dpp::snowflake user_id, SQLite::Database& db) {
    return try_insert(db, "user_settings", "setting_key",
                     static_cast<int64_t>(user_id)); // or use "user:" + std::to_string(user_id)
}

bool init_guild_user(dpp::snowflake guild_id, dpp::snowflake user_id, SQLite::Database& db) {
    std::string table_name = "guild_" + std::to_string(guild_id);
    db.exec("CREATE TABLE IF NOT EXISTS " + table_name + " ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "setting_key TEXT NOT NULL, setting_value TEXT, "
            "UNIQUE(setting_key))");
    return try_insert(db, table_name, "setting_key",
                     static_cast<int64_t>(user_id));
}

// Delete guild from guild_settings AND drop its per-guild table
void delete_guild(dpp::snowflake guild_id, SQLite::Database& db) {
    // Remove from guild_settings
    SQLite::Statement stmt(db, "DELETE FROM guild_settings WHERE guild_id = ?");
    stmt.bind(1, static_cast<int64_t>(guild_id));
    stmt.exec();

    // Drop per-guild table
    std::string table_name = "guild_" + std::to_string(guild_id);
    db.exec("DROP TABLE IF EXISTS " + table_name);
}

// Delete user from global user_settings
void delete_user(dpp::snowflake user_id, SQLite::Database& db) {
    std::string user_key = "user:" + std::to_string(user_id);
    SQLite::Statement stmt(db, "DELETE FROM user_settings WHERE setting_key = ?");
    stmt.bind(1, user_key);
    stmt.exec();
}

// Delete user from a specific guild's table
void delete_guild_user(dpp::snowflake guild_id, dpp::snowflake user_id, SQLite::Database& db) {
    std::string table_name = "guild_" + std::to_string(guild_id);
    std::string user_key = "user:" + std::to_string(user_id);
    SQLite::Statement stmt(db, "DELETE FROM " + table_name + " WHERE setting_key = ?");
    stmt.bind(1, user_key);
    stmt.exec();
}

// Set property for a user
void set_user_property(dpp::snowflake user_id, const std::string& key, 
                       const std::string& value, SQLite::Database& db) {
    std::string user_key = "user:" + std::to_string(user_id) + ":" + key;
    SQLite::Statement stmt(db, "INSERT OR REPLACE INTO user_settings "
                              "(setting_key, setting_value) VALUES (?, ?)");
    stmt.bind(1, user_key);
    stmt.bind(2, value);
    stmt.exec();
}

// Set property for a guild
void set_guild_property(dpp::snowflake guild_id, const std::string& key,
                       const std::string& value, SQLite::Database& db) {
    SQLite::Statement stmt(db, "INSERT OR REPLACE INTO guild_settings "
                              "(guild_id, " + key + ") VALUES (?, ?)");
    stmt.bind(1, static_cast<int64_t>(guild_id));
    stmt.bind(2, value);
    stmt.exec();
}

// Set property for a user within a guild
void set_guild_user_property(dpp::snowflake guild_id, dpp::snowflake user_id,
                            const std::string& key, const std::string& value,
                            SQLite::Database& db) {
    std::string table_name = "guild_" + std::to_string(guild_id);
    std::string user_key = "user:" + std::to_string(user_id) + ":" + key;

    SQLite::Statement stmt(db, "INSERT OR REPLACE INTO " + table_name + " "
                              "(setting_key, setting_value) VALUES (?, ?)");
    stmt.bind(1, user_key);
    stmt.bind(2, value);
    stmt.exec();
}