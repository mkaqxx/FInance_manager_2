# Слой хранения данных (Storage & Event Sourcing)

* **Что здесь будет:** 
  * `SqliteDb.hpp / .cpp` — RAII-обертка над SQLite3 (режим WAL, управление транзакциями).
  * `SchemaMigrations.hpp / .cpp` — DDL-схема таблиц (`accounts`, `envelopes`, `transactions`, `envelope_ledger`, `routing_rules`, `fx_rates`).
  * `Repositories/` — классы выборки и сохранения данных.
* **Для чего:** Локальное встраиваемое хранилище. Реализует Event Sourcing: в БД нет мутабельных колонок баланса, все остатки вычисляются на лету через `SUM(amount_minor)`.
