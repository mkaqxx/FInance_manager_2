# Статическая библиотека ядра
add_library(FinanceCore STATIC
        src/utils/UuidV7.cpp
        include/FinanceCoreTypes.hpp
        src/utils/Int128Math.hpp
        include/UuidV7.hpp
        src/storage/SqliteDb.cpp
        src/storage/SqliteDb.hpp
        src/storage/SqliteStatement.hpp
        src/storage/SqliteTransaction.cpp
        src/storage/SqliteTransaction.hpp
        src/storage/SchemaMigrations.cpp
        src/storage/SchemaMigrations.hpp
        src/domain/BudgetMath.cpp
        src/domain/BudgetMath.hpp
        src/domain/MoneyRouter.cpp
        src/domain/MoneyRouter.hpp
        src/domain/Reconciliation.cpp
        src/domain/Reconciliation.hpp
)

# Разделение публичных и приватных путей включения:
target_include_directories(FinanceCore
        PUBLIC
        include    # Публичный C++20 фасад (виден Swift на iOS и хосту Windows)
        PRIVATE
        src        # Внутренняя реализация (видна только внутри ядра)
)

# Линкуем ядро со сторонними библиотеками:
target_link_libraries(FinanceCore
        PUBLIC
        sqlite3
        nlohmann_json
        csv_parser
)

# Регистрация теста математики
add_executable(math_tests tests/test_math.cpp)
target_link_libraries(math_tests PRIVATE FinanceCore)
add_test(NAME MathTests COMMAND math_tests)

# Регистрация теста хранилища (SQLite3)
add_executable(storage_tests tests/test_storage.cpp)
target_link_libraries(storage_tests PRIVATE FinanceCore)
add_test(NAME StorageTests COMMAND storage_tests)
