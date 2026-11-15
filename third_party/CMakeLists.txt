# ------------------------------------------------------------------------------
# 1. Сборка SQLite3 как статической C-библиотеки с оптимизациями
# ------------------------------------------------------------------------------
add_library(sqlite3 STATIC
        sqlite3/sqlite3.c
)

target_include_directories(sqlite3 PUBLIC
        sqlite3
)

# Важнейшие макросы для SQLite из спецификации нашего проекта:
target_compile_definitions(sqlite3 PRIVATE
        SQLITE_ENABLE_JSON1             # Быстрый встроенный парсер JSON
        SQLITE_THREADSAFE=1            # Потокобезопасность
        SQLITE_DEFAULT_FOREIGN_KEYS=1  # Включение проверки связей таблиц по умолчанию
        SQLITE_ENABLE_SESSION          # Поддержка отслеживания изменений для дельт
        SQLITE_ENABLE_PREUPDATE_HOOK   # Хук для перехвата изменений перед записью
)

# ------------------------------------------------------------------------------
# 2. Регистрация Header-Only (заголовочных) библиотек
# ------------------------------------------------------------------------------

# nlohmann/json
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE json)

# vincentlaucsb/csv-parser
add_library(csv_parser INTERFACE)
target_include_directories(csv_parser INTERFACE csv-parser)

# cpp-httplib (нужен только для десктопа)
add_library(cpp_httplib INTERFACE)
target_include_directories(cpp_httplib INTERFACE cpp-httplib)