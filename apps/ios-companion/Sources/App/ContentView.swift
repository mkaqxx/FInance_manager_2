import SwiftUI

struct ContentView: View {
    @State private var keychainStatus: String = "Нажмите кнопку для проверки..."
    @State private var testData: String = ""
    @State private var urlMessage: String = "Ожидание вызова finance://..."

    var body: some View {
        NavigationStack {
            VStack(spacing: 20) {
                Image(systemName: "checkmark.shield.fill")
                    .font(.system(size: 56))
                    .foregroundStyle(.green)

                Text("Finance Companion")
                    .font(.title2.bold())

                VStack(alignment: .leading, spacing: 10) {
                    Text("1. Системный Keychain:")
                        .font(.headline)
                    Text(keychainStatus)
                        .font(.subheadline)
                        .foregroundStyle(.secondary)

                    if !testData.isEmpty {
                        Text("Snapshot из памяти:")
                            .font(.caption.bold())
                        Text(testData)
                            .font(.caption.monospaced())
                            .padding(8)
                            .frame(maxWidth: .infinity, alignment: .leading)
                            .background(Color(.secondarySystemBackground))
                            .cornerRadius(8)
                    }
                }
                .padding()
                .frame(maxWidth: .infinity, alignment: .leading)
                .background(Color(.systemBackground))
                .cornerRadius(12)
                .shadow(color: .black.opacity(0.06), radius: 4)

                VStack(alignment: .leading, spacing: 8) {
                    Text("2. Прямой URL-вызов из Shortcuts:")
                        .font(.headline)
                    Text(urlMessage)
                        .font(.subheadline)
                        .foregroundStyle(.blue)
                }
                .padding()
                .frame(maxWidth: .infinity, alignment: .leading)
                .background(Color(.systemBackground))
                .cornerRadius(12)
                .shadow(color: .black.opacity(0.06), radius: 4)

                Button("Проверить запись и чтение Keychain") {
                    runKeychainTest()
                }
                .buttonStyle(.borderedProminent)
            }
            .padding()
            .navigationTitle("Тест хранилища")
            .onAppear {
                runKeychainTest()
            }
            .onOpenURL { url in
                handleIncomingURL(url)
            }
        }
    }

    private func runKeychainTest() {
        let timestamp = ISO8601DateFormatter().string(from: Date())
        let sampleJSON = "{\"safe_limit_byn\": 45.50, \"status\": \"OK\", \"time\": \"\(timestamp)\"}"
        guard let data = sampleJSON.data(using: .utf8) else { return }

        if KeychainHelper.save(key: "daily_snapshot", data: data) {
            if let loaded = KeychainHelper.load(key: "daily_snapshot"),
               let str = String(data: loaded, encoding: .utf8) {
                keychainStatus = "✅ Keychain работает на 100%! Данные сохранены и прочитаны."
                testData = str
            } else {
                keychainStatus = "⚠️ Запись прошла, но ошибка при чтении."
            }
        } else {
            keychainStatus = "❌ Ошибка записи в Keychain."
        }
    }

    private func handleIncomingURL(_ url: URL) {
        urlMessage = "✅ Получен вызов: \(url.absoluteString)"
    }
}
