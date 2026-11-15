# Windows Host (Десктопное приложение)

* **Что здесь будет:**
  * `WinMain.cpp` — точка входа Windows-приложения, создание окна Win32 и монтирование Microsoft Edge WebView2.
  * `RpcDispatcher.hpp / .cpp` — асинхронный JSON-RPC мост между JS в браузере и C++ ядром.
  * `DownloadsWatcher.hpp / .cpp` — фоновый наблюдатель за папкой `Downloads` на базе Win32 API `ReadDirectoryChangesW`.
  * `NbrbRateClient.hpp / .cpp` — клиент загрузки курсов валют НБ РБ через WinHTTP (раз в 12 часов).
  * `LocalSyncServer.hpp / .cpp` — локальный HTTP/JSON-сервер на `cpp-httplib` (порт `:8443`) со спариванием по QR-коду.
* **Для чего:** Десктопный интерфейс и системные службы Windows без оверхеда Electron (~40 МБ ОЗУ).
