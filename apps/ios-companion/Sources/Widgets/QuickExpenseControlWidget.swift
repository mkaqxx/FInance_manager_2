import AppIntents
import SwiftUI
import WidgetKit

struct QuickExpenseControlWidget: ControlWidget {
    var body: some ControlWidgetConfiguration {
        StaticControlConfiguration(kind: "com.mkaqxx.FinanceCompanion.QuickExpense") {
            ControlWidgetButton(action: LogQuickExpenseIntent()) {
                Label("Расход", systemImage: "creditcard.fill")
            }
        }
        .displayName("Быстрый расход")
        .description("Ввод суммы повседневного расхода.")
    }
}

struct LogQuickExpenseIntent: AppIntent {
    static let title: LocalizedStringResource = "Записать расход"
    static let description = IntentDescription("Запрашивает сумму и сохраняет событие в inbox")

    @Parameter(title: "Сумма (BYN)", description: "Сумма операции")
    var amount: Double?

    static var parameterSummary: some ParameterSummary {
        Summary("Записать расход на \(\.$amount) BYN")
    }

    func perform() async throws -> some IntentResult & ProvidesDialog {
        let enteredAmount: Double
        if let a = amount {
            enteredAmount = a
        } else {
            enteredAmount = try await $amount.requestValue("Введите сумму расхода (BYN):")
        }

        let appGroupId = "group.com.mkaqxx.finance"
        if let containerURL = FileManager.default.containerURL(forSecurityApplicationGroupIdentifier: appGroupId) {
            let inboxDir = containerURL.appendingPathComponent("inbox", isDirectory: true)
            try? FileManager.default.createDirectory(at: inboxDir, withIntermediateDirectories: true)
            let fileURL = inboxDir.appendingPathComponent("\(UUID().uuidString).json")
            let timestamp = ISO8601DateFormatter().string(from: Date())
            let payload = "{\"amount\": \(enteredAmount), \"date\": \"\(timestamp)\"}"
            try? payload.write(to: fileURL, atomically: true, encoding: .utf8)
        }

        return .result(dialog: "Расход \(String(format: "%.2f", enteredAmount)) BYN записан!")
    }
}
