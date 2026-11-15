# Сторонние библиотеки (Vendored Dependencies)

* **Что здесь находится:**
  * `sqlite3/` (`sqlite3.c`, `sqlite3.h`) — встраиваемая СУБД с режимом WAL и PRAGMA foreign_keys = ON.
  * `json/nlohmann/` (`json.hpp`) — библиотека работы с JSON.
  * `csv-parser/` (`csv.hpp`) — потоковый высокоскоростной парсер CSV.
  * `cpp-httplib/` (`httplib.h`) — однофайловый C++ HTTP-сервер для P2P Wi-Fi синхронизации на Windows.
* **Для чего:** Обеспечение 100% автономности сборки без vcpkg, conan и внешних пакетных менеджеров.
