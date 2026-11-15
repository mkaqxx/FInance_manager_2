# Статическая библиотека ядра
add_library(FinanceCore STATIC
        # Пока ядро пустое, добавляем заглушку, чтобы CMake не ругался на отсутствие файлов
        src/utils/UuidV7.cpp
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