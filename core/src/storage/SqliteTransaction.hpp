#pragma once
#include <sqlite3.h>
#include <string>
#include <expected>
#include <utility>

namespace finance::storage {

    class SqliteTransaction {
    public:
        // Запрещаем копирование
        SqliteTransaction(const SqliteTransaction&) = delete;
        SqliteTransaction& operator=(const SqliteTransaction&) = delete;

        // Перемещение (конструктор разрешен, присваивание удалено)
        SqliteTransaction(SqliteTransaction&& other) noexcept
            : db_(std::exchange(other.db_, nullptr)),
              completed_(std::exchange(other.completed_, true)) {}
        SqliteTransaction& operator=(SqliteTransaction&&) = delete;

        // Деструктор: откат по умолчанию
        ~SqliteTransaction() {
            rollback();
        }

        // Методы фиксации и отката
        std::expected<void, std::string> commit();
        void rollback() noexcept;

    private:
        friend class SqliteDb;

        explicit SqliteTransaction(sqlite3* db) noexcept : db_(db) {}

        sqlite3* db_{nullptr};
        bool completed_{false};
    };

} // namespace finance::storage