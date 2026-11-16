# Статическая библиотека ядра
add_library(FinanceCore STATIC
        # Пока ядро пустое, добавляем заглушку, чтобы CMake не ругался на отсутствие файлов
        src/utils/UuidV7.cpp
        include/FinanceCoreTypes.hpp
        src/utils/Int128Math.hpp
        tests/test_math.cpp
        include/UuidV7.hpp
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