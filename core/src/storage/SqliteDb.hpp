#pragma once
#include <sqlite3.h>
#include <string>
#include <filesystem>
#include <expected>
#include <utility>
#include "SqliteStatement.hpp"
#include "SqliteTransaction.hpp"


namespace finance::storage {
    class SqliteDb {
    public:
        SqliteDb(const SqliteDb& ) = delete;
        SqliteDb& operator =( const SqliteDb& ) = delete;

        SqliteDb(SqliteDb&& other ) noexcept : db_(std::exchange(other.db_, nullptr)) {}
        SqliteDb& operator = (SqliteDb&& other ) noexcept;

        ~SqliteDb() { close(); }

        static std::expected<SqliteDb, std::string> openFile (const std::filesystem::path& path);
        static std::expected<SqliteDb, std::string> openInMemory();

        [[nodiscard]] sqlite3* handle() const noexcept {return db_;}
        [[nodiscard]] bool isOpen() const noexcept {return db_ != nullptr;}

        void close() noexcept;

        std::expected<void, std::string> execute(std::string_view sql);
        std::expected<SqliteStatement, std::string> prepare(std::string_view sql);
        std::expected<SqliteTransaction, std::string> beginTransaction();
    private:
        explicit SqliteDb(sqlite3* db) noexcept : db_(db) {}
        sqlite3* db_{nullptr};
    };
}
