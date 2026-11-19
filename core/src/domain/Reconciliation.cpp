#include "Reconciliation.hpp"
#include "UuidV7.hpp"


namespace finance {

    ReconciliationPlan reconcileBalance(const std::string& account_id, MoneyMinor actual_balance_minor,
        MoneyMinor computed_balance_minor, int64_t timestamp_utc, std::string_view local_date) noexcept{
        MoneyMinor discrepancy = calculateDiscrepancy(actual_balance_minor, computed_balance_minor);
        if (discrepancy == 0) {
            return ReconciliationPlan({0, ""}, std::nullopt);
        }
        std::string tx_id = generateUuidV7();
        TransactionDto tx (tx_id, account_id, "",timestamp_utc,
            std::string(local_date), discrepancy,"сверка баланса",
            "сверка баланса", 0, TransactionStatus::RECONCILED);
        return {{discrepancy, tx_id}, tx};
    }
}