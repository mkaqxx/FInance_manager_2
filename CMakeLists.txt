cmake_minimum_required(VERSION 3.28)
project(FinanceManager VERSION 2.0.0 LANGUAGES C CXX)

# Требуем современный стандарт C++23 для всего проекта
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Обязательный флаг /utf-8 для компилятора MSVC (Visual Studio):
# защищает от битых русских символов в кодировках и логах
if (MSVC)
    add_compile_options(/utf-8 /W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Включаем модульное тестирование ctest
enable_testing()

# Подключаем модули в строгом порядке зависимостей
add_subdirectory(third_party)
add_subdirectory(core)