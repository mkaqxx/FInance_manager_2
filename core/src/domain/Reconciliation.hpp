#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <optional>
#include "FinanceCoreTypes.hpp"
#include "UuidV7.hpp"

namespace finance {

    // План сверки: результат и готовая к сохранению транзакция (если расхождение есть)
    struct ReconciliationPlan {
        ReconciliationResult result;
        std::optional<TransactionDto> transaction;
    };

    // 1. Быстрый расчет дельты расхождения
    [[nodiscard]] constexpr MoneyMinor calculateDiscrepancy( MoneyMinor actual_balance_minor, MoneyMinor computed_balance_minor ) noexcept {
        return actual_balance_minor - computed_balance_minor;
    }

    // 2. Создание транзакции сверки
    [[nodiscard]] ReconciliationPlan reconcileBalance(const std::string& account_id, MoneyMinor actual_balance_minor,
        MoneyMinor computed_balance_minor, int64_t timestamp_utc, std::string_view local_date) noexcept;

} // namespace finance