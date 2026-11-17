#include "SqliteDb.hpp"


namespace finance::storage {
    SqliteDb& SqliteDb::operator =( SqliteDb&& other ) noexcept {
        if (this != &other) {
            close();
            db_=std::exchange(other.db_, nullptr);
        }
        return *this;
    }


    void SqliteDb::close() noexcept {
        if (db_) {
            sqlite3_close_v2(db_);
            db_ = nullptr;
        }
    }


    std::expected<SqliteDb, std::string> SqliteDb::openFile (const std::filesystem::path& path) {
        sqlite3* rawDb = nullptr;
        int rc = sqlite3_open_v2(reinterpret_cast<const char*>(path.u8string().c_str()), &rawDb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
        if (rc != SQLITE_OK) {
            std::string errorMsg = rawDb ? sqlite3_errmsg(rawDb) : "Failed to open database";
            if (rawDb) {
                sqlite3_close_v2(rawDb);
            }
            return std::unexpected(errorMsg);
        }
        SqliteDb temp = SqliteDb(rawDb);
        auto res = temp.execute(
        "PRAGMA foreign_keys = ON; "
            "PRAGMA journal_mode = WAL; "
            "PRAGMA busy_timeout = 5000; "
            "PRAGMA synchronous = NORMAL;");
        if (!res) {
            return std::unexpected(res.error());
        }
        return temp;
    }

    std::expected<SqliteDb, std::string> SqliteDb::openInMemory() {
        return openFile(":memory:");
    }


    std::expected<void, std::string> SqliteDb::execute(std::string_view sql) {
        if (!db_) return std::unexpected("Databased is closed!");
      int rc =  sqlite3_exec(db_, std::string(sql).c_str(), nullptr, nullptr, nullptr);
        if (rc != SQLITE_OK) {
            std::string errorMsg = sqlite3_errmsg(db_);
            return std::unexpected(errorMsg);
        }
        return {};
    }


    std::expected<SqliteStatement, std::string> SqliteDb::prepare(std::string_view sql) {
        if (!db_) return std::unexpected("Database is closed!");
        sqlite3_stmt* rawStmt = nullptr;
        int rc = sqlite3_prepare_v2(db_, sql.data(), static_cast<int>(sql.size()), &rawStmt, nullptr);
        if (rc != SQLITE_OK) {
            std::string errorMsg = sqlite3_errmsg(db_);
            return std::unexpected(errorMsg);
        }
        return SqliteStatement(rawStmt);
    }

    std::expected<SqliteTransaction, std::string> SqliteDb::beginTransaction() {
        if (!db_) return std::unexpected("Database is closed!");
        auto res = execute("BEGIN IMMEDIATE;");
        if (!res) {
            return std::unexpected(res.error());
        }
        return SqliteTransaction(db_);
    }
}



