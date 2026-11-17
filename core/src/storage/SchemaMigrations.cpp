#include "SchemaMigrations.hpp"


namespace finance::storage {
    std::expected<void, std::string> SchemaMigrations::apply(SqliteDb &db) {
        auto tx = db.beginTransaction();
        if (!tx) return std::unexpected{tx.error()};
        constexpr std::string_view kSchemaSql = R"(
            -- 1. Таблица счетов (карты, кошельки, вклады)
            CREATE TABLE IF NOT EXISTS accounts (
                id TEXT PRIMARY KEY,
                name TEXT NOT NULL,
                currency TEXT NOT NULL,
                is_archived INTEGER NOT NULL DEFAULT 0,
                created_at INTEGER NOT NULL
            );

            -- 2. Таблица конвертов / финансовых целей
            CREATE TABLE IF NOT EXISTS envelopes (
                id TEXT PRIMARY KEY,
                name TEXT NOT NULL,
                target_amount INTEGER NOT NULL DEFAULT 0,
                kind INTEGER NOT NULL,
                priority INTEGER NOT NULL,
                currency TEXT NOT NULL,
                created_at INTEGER NOT NULL
            );

            -- 3. Таблица транзакций (лента неизменяемых событий Event Sourcing)
            CREATE TABLE IF NOT EXISTS transactions (
                id TEXT PRIMARY KEY,
                account_id TEXT NOT NULL REFERENCES accounts(id) ON DELETE RESTRICT,
                amount_minor INTEGER NOT NULL,
                currency TEXT NOT NULL,
                status TEXT NOT NULL,
                timestamp INTEGER NOT NULL,
                merchant TEXT,
                note TEXT,
                raw_payload TEXT
            );

            -- Индексы для мгновенного поиска по транзакциям
            CREATE INDEX IF NOT EXISTS idx_transactions_account ON transactions(account_id);
            CREATE INDEX IF NOT EXISTS idx_transactions_timestamp ON transactions(timestamp);

            -- 4. Бухгалтерская книга конвертов (проводки по целям)
            CREATE TABLE IF NOT EXISTS envelope_ledger (
                id TEXT PRIMARY KEY,
                transaction_id TEXT REFERENCES transactions(id) ON DELETE CASCADE,
                envelope_id TEXT NOT NULL REFERENCES envelopes(id) ON DELETE RESTRICT,
                amount_minor INTEGER NOT NULL,
                timestamp INTEGER NOT NULL
            );

            CREATE INDEX IF NOT EXISTS idx_envelope_ledger_env ON envelope_ledger(envelope_id);

            -- 5. Правила маршрутизатора входящих денег
            CREATE TABLE IF NOT EXISTS routing_rules (
                id TEXT PRIMARY KEY,
                priority INTEGER NOT NULL,
                rule_type TEXT NOT NULL,
                value INTEGER NOT NULL,
                target_envelope_id TEXT NOT NULL REFERENCES envelopes(id) ON DELETE CASCADE
            );

            -- 6. Курсы валют
            CREATE TABLE IF NOT EXISTS fx_rates (
                from_currency TEXT NOT NULL,
                to_currency TEXT NOT NULL,
                rate_micro INTEGER NOT NULL,
                timestamp INTEGER NOT NULL,
                PRIMARY KEY (from_currency, to_currency)
            );
        )";
        auto exeRes = db.execute(kSchemaSql);
        if (!exeRes) return std::unexpected(exeRes.error());
        auto commitRes = tx->commit();
        if (!commitRes) return std::unexpected(commitRes.error());
        return {};
    }
}
