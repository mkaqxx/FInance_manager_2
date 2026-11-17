#pragma once
#include "SqliteDb.hpp"
#include <expected>
#include <string>

namespace finance::storage {

    class SchemaMigrations {
    public:
        // Накатывает схему базы данных (создает таблицы и индексы, если их еще нет)
        static std::expected<void, std::string> apply(SqliteDb& db);
    };

} // namespace finance::storage