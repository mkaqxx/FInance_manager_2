#pragma once
#include <sqlite3.h>
#include <string>
#include <expected>
#include <utility>


namespace finance::storage {
    enum class StepResult { Row, Done };

    class SqliteStatement {
        public:

        SqliteStatement(const SqliteStatement&) = delete;
        SqliteStatement& operator=(const SqliteStatement&) = delete;

        SqliteStatement(SqliteStatement&& other) noexcept : stmt_(std::exchange(other.stmt_, nullptr)) {}
        SqliteStatement& operator=(SqliteStatement&& other) noexcept {
            if (this != &other) {
                finalize();
                stmt_ = std::exchange(other.stmt_, nullptr);
            }
            return *this;
        }

        ~SqliteStatement() {finalize();}
        void finalize() noexcept {
            if (stmt_) {
                sqlite3_finalize(stmt_);
                stmt_ = nullptr;
            }
        }
        sqlite3_stmt* handle() const noexcept { return stmt_; }


        std::expected<void, std::string> bind(int index, int64_t value) {
            if (!stmt_) return std::unexpected("Statement is closed or invalid");
            int rc = sqlite3_bind_int64(stmt_, index, value);
            if (rc != SQLITE_OK) {
                return std::unexpected(sqlite3_errstr(rc));
            }
            return {};
        }
        std::expected<void, std::string> bind(int index, double value) {
            if (!stmt_) return std::unexpected("Statement is closed or invalid");
            int rc = sqlite3_bind_double(stmt_, index, value);
            if (rc != SQLITE_OK) {
                return std::unexpected(sqlite3_errstr(rc));
            }
            return {};
        }
        std::expected<void, std::string> bind(int index, std::string_view value) {
            if (!stmt_) return std::unexpected("Statement is closed or invalid");
            int rc = sqlite3_bind_text(stmt_, index, value.data(), static_cast<int>(value.size()), SQLITE_TRANSIENT);
            if (rc != SQLITE_OK) {
                return std::unexpected(sqlite3_errstr(rc));
            }
            return {};
        }
        std::expected<void, std::string> bindNull(int index) {
            if (!stmt_) return std::unexpected("Statement is closed or invalid");
            int rc = sqlite3_bind_null(stmt_, index);
            if (rc != SQLITE_OK) {
                return std::unexpected(sqlite3_errstr(rc));
            }
            return {};
        }

        std::expected<StepResult, std::string> step() {
            if (!stmt_) return std::unexpected("Statement is closed or invalid");
            int rc = sqlite3_step(stmt_);
            if (rc == SQLITE_DONE) return StepResult::Done;
            if (rc == SQLITE_ROW) return StepResult::Row;
            return std::unexpected(sqlite3_errstr(rc));
        }

        void reset() noexcept {
            if (stmt_) {
                sqlite3_reset(stmt_);
                sqlite3_clear_bindings(stmt_);
            }
        }

        [[nodiscard]] int64_t getInt64(int col) const noexcept {
            return sqlite3_column_int64(stmt_, col);
        }

        [[nodiscard]] double getDouble(int col) const noexcept {
            return sqlite3_column_double(stmt_, col);
        }

        [[nodiscard]] std::string getString(int col) const {
            const auto* text = sqlite3_column_text(stmt_, col);
            if (!text) return "";
            int bytes = sqlite3_column_bytes(stmt_, col);
            return std::string(reinterpret_cast<const char*>(text), bytes);
        }

        [[nodiscard]] bool isNull(int col) const noexcept {
            return sqlite3_column_type(stmt_, col) == SQLITE_NULL;
        }

        private:
        friend class SqliteDb;
        sqlite3_stmt* stmt_{nullptr};
        explicit SqliteStatement(sqlite3_stmt* stmt) noexcept : stmt_(stmt) {}
    };
}