#include "../src/storage/SqliteDb.hpp"
#include "../src/storage/SchemaMigrations.hpp"
#include <iostream>
#include <cassert>

void test_open_and_execute() {
    auto dbRes = finance::storage::SqliteDb::openInMemory();
    assert(dbRes.has_value());
    auto db = std::move(dbRes.value());
    assert(db.isOpen());

    auto createRes = db.execute(
        "CREATE TABLE test_accounts ("
        "id TEXT PRIMARY KEY, "
        "name TEXT NOT NULL, "
        "balance INTEGER NOT NULL"
        ");"
    );
    assert(createRes.has_value());
    std::cout << "[PASS] Table created successfully\n";
}

void test_bind_and_insert() {
    auto db = finance::storage::SqliteDb::openInMemory().value();
    db.execute("CREATE TABLE accounts (id TEXT PRIMARY KEY, name TEXT, balance INTEGER);");

    auto stmtRes = db.prepare("INSERT INTO accounts (id, name, balance) VALUES (?, ?, ?);");
    assert(stmtRes.has_value());
    auto stmt = std::move(stmtRes.value());

    // Row 1
    assert(stmt.bind(1, "acc-1").has_value());
    assert(stmt.bind(2, "Основной счет").has_value());
    assert(stmt.bind(3, int64_t{50000}).has_value());
    auto step1 = stmt.step();
    assert(step1.has_value() && step1.value() == finance::storage::StepResult::Done);

    // Row 2 (Кавычки в строке для проверки безопасности!)
    stmt.reset();
    assert(stmt.bind(1, "acc-2").has_value());
    assert(stmt.bind(2, "O'Reilly & McDonald's").has_value());
    assert(stmt.bind(3, int64_t{12345}).has_value());
    auto step2 = stmt.step();
    assert(step2.has_value() && step2.value() == finance::storage::StepResult::Done);

    std::cout << "[PASS] Prepared statement insert with quotes passed\n";
}

void test_select() {
    auto db = finance::storage::SqliteDb::openInMemory().value();
    db.execute("CREATE TABLE accounts (id TEXT PRIMARY KEY, name TEXT, balance INTEGER);");

    auto insert = db.prepare("INSERT INTO accounts (id, name, balance) VALUES (?, ?, ?);").value();
    insert.bind(1, "acc-1");
    insert.bind(2, "Checking");
    insert.bind(3, int64_t{1000});
    insert.step();

    auto selectRes = db.prepare("SELECT id, name, balance FROM accounts WHERE id = ?;");
    assert(selectRes.has_value());
    auto select = std::move(selectRes.value());
    select.bind(1, "acc-1");

    auto step = select.step();
    assert(step.has_value() && step.value() == finance::storage::StepResult::Row);
    assert(select.getString(0) == "acc-1");
    assert(select.getString(1) == "Checking");
    assert(select.getInt64(2) == 1000);

    auto stepEnd = select.step();
    assert(stepEnd.has_value() && stepEnd.value() == finance::storage::StepResult::Done);

    std::cout << "[PASS] SELECT reading columns passed\n";
}

void test_move() {
    auto db1 = finance::storage::SqliteDb::openInMemory().value();
    auto db2 = std::move(db1);
    assert(!db1.isOpen());
    assert(db2.isOpen());
    std::cout << "[PASS] Move semantics passed\n";
}

void test_transactions() {
    auto db = finance::storage::SqliteDb::openInMemory().value();
    db.execute("CREATE TABLE users (id TEXT PRIMARY KEY, name TEXT);");

    // 1. Тест успешного COMMIT
    {
        auto tx = db.beginTransaction().value();
        auto stmt = db.prepare("INSERT INTO users (id, name) VALUES (?, ?);").value();
        stmt.bind(1, "u1");
        stmt.bind(2, "Alice");
        stmt.step();
        assert(tx.commit().has_value());
    }

    // Проверяем, что Alice сохранилась
    {
        auto select = db.prepare("SELECT name FROM users WHERE id = 'u1';").value();
        assert(select.step().value() == finance::storage::StepResult::Row);
        assert(select.getString(0) == "Alice");
    }

    // 2. Тест автоматического ROLLBACK в деструкторе (вышли без commit)
    {
        auto tx = db.beginTransaction().value();
        auto stmt = db.prepare("INSERT INTO users (id, name) VALUES (?, ?);").value();
        stmt.bind(1, "u2");
        stmt.bind(2, "Bob");
        stmt.step();
        // ВНИМАНИЕ: не вызываем tx.commit()! Скоуп заканчивается прямо здесь!
    }

    // Проверяем, что Bob НЕ сохранился (деструктор tx откатил транзакцию)
    {
        auto select = db.prepare("SELECT name FROM users WHERE id = 'u2';").value();
        assert(select.step().value() == finance::storage::StepResult::Done); // Нет строк!
    }

    // 3. Тест явного ROLLBACK
    {
        auto tx = db.beginTransaction().value();
        auto stmt = db.prepare("INSERT INTO users (id, name) VALUES (?, ?);").value();
        stmt.bind(1, "u3");
        stmt.bind(2, "Charlie");
        stmt.step();
        tx.rollback();
    }

    // Проверяем, что Charlie тоже нет
    {
        auto select = db.prepare("SELECT name FROM users WHERE id = 'u3';").value();
        assert(select.step().value() == finance::storage::StepResult::Done);
    }

    std::cout << "[PASS] Transactions (COMMIT & automatic ROLLBACK) passed\n";
}

void test_schema_migrations() {
    auto db = finance::storage::SqliteDb::openInMemory().value();

    // 1. Применяем миграции
    auto applyRes = finance::storage::SchemaMigrations::apply(db);
    assert(applyRes.has_value());

    // 2. Проверяем, что все 6 таблиц созданы в sqlite_master
    {
        auto checkStmt = db.prepare(
            "SELECT count(*) FROM sqlite_master WHERE type='table' AND name IN "
            "('accounts', 'envelopes', 'transactions', 'envelope_ledger', 'routing_rules', 'fx_rates');"
        ).value();
        assert(checkStmt.step().value() == finance::storage::StepResult::Row);
        assert(checkStmt.getInt64(0) == 6);
    }

    // 3. Проверяем идемпотентность (повторный вызов не должен ломать базу)
    auto applyAgainRes = finance::storage::SchemaMigrations::apply(db);
    assert(applyAgainRes.has_value());

    // 4. Проверяем работу ВНЕШНИХ КЛЮЧЕЙ (Foreign Keys):
    // Попытка вставить транзакцию с несуществующим account_id должна провалиться!
    {
        auto badTx = db.prepare(
            "INSERT INTO transactions (id, account_id, amount_minor, currency, status, timestamp) "
            "VALUES (?, ?, ?, ?, ?, ?);"
        ).value();
        badTx.bind(1, "tx_ghost");
        badTx.bind(2, "non_existent_account"); // Нет такого счета!
        badTx.bind(3, int64_t{-1000});
        badTx.bind(4, "BYN");
        badTx.bind(5, "CLEARED");
        badTx.bind(6, int64_t{1700000000});
        auto badStep = badTx.step();
        assert(!badStep.has_value()); // Ошибка Foreign Key!
    }

    // 5. Создаем реальный счет и добавляем транзакции (Event Sourcing)
    {
        auto acc = db.prepare("INSERT INTO accounts (id, name, currency, created_at) VALUES (?, ?, ?, ?);").value();
        acc.bind(1, "acc_main");
        acc.bind(2, "Основная карта");
        acc.bind(3, "BYN");
        acc.bind(4, int64_t{1700000000});
        assert(acc.step().value() == finance::storage::StepResult::Done);

        // Доход: +100.00 BYN (10000 копеек)
        auto tx1 = db.prepare(
            "INSERT INTO transactions (id, account_id, amount_minor, currency, status, timestamp, merchant) "
            "VALUES (?, ?, ?, ?, ?, ?, ?);"
        ).value();
        tx1.bind(1, "tx_income");
        tx1.bind(2, "acc_main");
        tx1.bind(3, int64_t{10000});
        tx1.bind(4, "BYN");
        tx1.bind(5, "CLEARED");
        tx1.bind(6, int64_t{1700000001});
        tx1.bind(7, "Зарплата");
        assert(tx1.step().value() == finance::storage::StepResult::Done);

        // Расход: -35.50 BYN (-3550 копеек)
        auto tx2 = db.prepare(
            "INSERT INTO transactions (id, account_id, amount_minor, currency, status, timestamp, merchant) "
            "VALUES (?, ?, ?, ?, ?, ?, ?);"
        ).value();
        tx2.bind(1, "tx_coffee");
        tx2.bind(2, "acc_main");
        tx2.bind(3, int64_t{-3550});
        tx2.bind(4, "BYN");
        tx2.bind(5, "CLEARED");
        tx2.bind(6, int64_t{1700000002});
        tx2.bind(7, "Кафетерий");
        assert(tx2.step().value() == finance::storage::StepResult::Done);

        // 6. Проверяем расчет баланса Event Sourcing: SUM(amount_minor)
        auto balStmt = db.prepare("SELECT SUM(amount_minor) FROM transactions WHERE account_id = ?;").value();
        balStmt.bind(1, "acc_main");
        assert(balStmt.step().value() == finance::storage::StepResult::Row);
        // 10000 - 3550 = 6450 копеек (64.50 BYN)
        assert(balStmt.getInt64(0) == 6450);
    }

    // 7. Проверяем ON DELETE RESTRICT:
    // Нельзя удалить счет, пока у него есть транзакции в ленте!
    {
        auto delStmt = db.prepare("DELETE FROM accounts WHERE id = ?;").value();
        delStmt.bind(1, "acc_main");
        auto delStep = delStmt.step();
        assert(!delStep.has_value()); // Ошибка RESTRICT!
    }

    std::cout << "[PASS] SchemaMigrations (6 tables, Foreign Keys, Event Sourcing SUM) passed\n";
}

int main() {
    std::cout << "--- Running Storage Tests ---" << std::endl;
    test_open_and_execute();
    test_bind_and_insert();
    test_select();
    test_move();
    test_transactions();
    test_schema_migrations();
    std::cout << "--- ALL STORAGE TESTS PASSED! ---" << std::endl;
    return 0;
}
