import SwiftUI

struct ContentView: View {
    @State private var appGroupStatus: String = "Проверка App Group..."
    @State private var testData: String = ""

    let appGroupId = "group.com.mkaqxx.finance"

    var body: some View {
        NavigationStack {
            VStack(spacing: 24) {
                Image(systemName: "banknote.fill")
                    .font(.system(size: 64))
                    .foregroundStyle(.green)

                Text("Finance Companion")
                    .font(.title2.bold())
                
                Text("Тестовая сборка Этапа 0")
                    .font(.subheadline)
                    .foregroundStyle(.secondary)

                VStack(alignment: .leading, spacing: 12) {
                    Text("Статус App Group:")
                        .font(.headline)
                    Text(appGroupStatus)
                        .font(.footnote)
                        .foregroundStyle(.secondary)

                    if !testData.isEmpty {
                        Divider()
                        Text("Тестовый Snapshot:")
                            .font(.headline)
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
                .shadow(color: .black.opacity(0.08), radius: 6)

                Button("Записать тестовый Snapshot") {
                    writeTestSnapshot()
                }
                .buttonStyle(.borderedProminent)

                Button("Обновить данные") {
                    checkAppGroup()
                }
                .buttonStyle(.bordered)
            }
            .padding()
            .navigationTitle("Автопилот")
            .onAppear {
                checkAppGroup()
            }
        }
    }

    private func checkAppGroup() {
        guard let containerURL = FileManager.default.containerURL(forSecurityApplicationGroupIdentifier: appGroupId) else {
            appGroupStatus = "❌ Ошибка: контейнер App Group недоступен.\n(При установке через бесплатный Apple ID в Sideloadly убедитесь, что App Group включен)."
            return
        }
        appGroupStatus = "✅ App Group доступна:\n\(containerURL.path)"
        let fileURL = containerURL.appendingPathComponent("daily_snapshot.json")
        if let data = try? Data(contentsOf: fileURL),
           let str = String(data: data, encoding: .utf8) {
            testData = str
        } else {
            testData = "Файл daily_snapshot.json пока не создан."
        }
    }

    private func writeTestSnapshot() {
        guard let containerURL = FileManager.default.containerURL(forSecurityApplicationGroupIdentifier: appGroupId) else { return }
        let fileURL = containerURL.appendingPathComponent("daily_snapshot.json")
        let timestamp = ISO8601DateFormatter().string(from: Date())
        let sample = "{\"safe_limit_byn\": 45.50, \"status\": \"ACTIVE\", \"updated_at\": \"\(timestamp)\"}"
        try? sample.write(to: fileURL, atomically: true, encoding: .utf8)
        checkAppGroup()
    }
}
