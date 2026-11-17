#include "SqliteTransaction.hpp"


namespace finance::storage{
    std::expected<void, std::string>  SqliteTransaction::commit() {
        if (completed_) return std::unexpected("Transaction already completed");
        if (!db_) return std::unexpected("Database is null");
        if ( sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK) {
            return std::unexpected(sqlite3_errmsg(db_));
        }
        completed_ = true;
        return {};

    }

    void SqliteTransaction::rollback() noexcept {
        if (!completed_ && db_) {
            sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
            completed_ = true;
        }
    }
}