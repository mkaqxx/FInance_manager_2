#include "../src/utils/Int128Math.hpp"
#include "../include/UuidV7.hpp"
#include "../src/domain/BudgetMath.hpp"
#include "../src/domain/MoneyRouter.hpp"
#include "../src/domain/Reconciliation.hpp"
#include <iostream>
#include <cassert>
#include <set>


void test_rounding() {
    assert(finance::multiplyDivide(5, 1, 2) == 2);
    assert(finance::multiplyDivide(7, 1, 2) == 4);
    assert(finance::multiplyDivide(10050, 3350000, 1000000) == 33668);
    std::cout << "Rounding tests passed!" << std::endl;
}


void test_currency_conversion() {
    assert(finance::convertCurrency(10000, 3350000, 0, true) == 33500);
    assert(finance::convertCurrency(10000, 3350000, 150, true) == 34002);
    std::cout << "Currency conversion tests passed!" << std::endl;
}


void test_uuid_v7() {
    std::string id1 = finance::generateUuidV7();
    std::string id2 = finance::generateUuidV7();
    std::cout << "Generated UUID 1: " << id1 << std::endl;
    std::cout << "Generated UUID 2: " << id2 << std::endl;
    assert(!id1.empty());
    assert(id1 != id2); // Проверка на уникальность
    std::cout << "UUIDv7 tests passed!" << std::endl;
}


void test_budget_alpha() {
    using namespace finance;

    // 1. Мягких целей нет (<= 0) -> 10000 (100%)
    assert(calculateAlpha(1000, 0) == 10000);
    assert(calculateAlpha(1000, -500) == 10000);

    // 2. Денег нет или минус (<= 0) -> 0 (0%)
    assert(calculateAlpha(0, 1000) == 0);
    assert(calculateAlpha(-500, 1000) == 0);

    // 3. Свободных денег >= мягких целей -> 10000 (100%)
    assert(calculateAlpha(1000, 1000) == 10000);
    assert(calculateAlpha(1500, 1000) == 10000);

    // 4. Пропорциональное сжатие
    // 500 из 1000 = 50% -> 5000 bps
    assert(calculateAlpha(500, 1000) == 5000);
    // 250 из 1000 = 25% -> 2500 bps
    assert(calculateAlpha(250, 1000) == 2500);
    // 100 из 300 = 33.33% -> 3333 bps (банковское округление)
    assert(calculateAlpha(100, 300) == 3333);

    std::cout << "Budget alpha tests passed!" << std::endl;
}


void test_budget_safe_limit() {
    using namespace finance;

    // 1. Штатный сценарий (Happy path)
    {
        BudgetParams params{
            .liquid_balance_minor = 100'000,   // 1000.00
            .locked_envelopes_minor = 20'000,  // 200.00
            .spent_today_minor = 1'000,        // 10.00
            .mandatory_unpaid_minor = 10'000,  // 100.00
            .hard_goals_minor = 5'000,         // 50.00
            .soft_goals_minor = 10'000,        // 100.00
            .days_remaining = 5
        };
        // Free start = 100000 - 20000 + 1000 = 81000
        // Pool before soft = 81000 - 10000 - 5000 = 66000
        // Alpha: 66000 >= 10000 -> 10000 bps
        // Net pool = 66000 - 10000 = 56000
        // Quota = 56000 / 5 = 11200
        // Limit today = 11200 - 1000 = 10200
        SafeLimitResult res = calculateSafeLimit(params);
        assert(!res.is_deficit);
        assert(res.alpha_bps == 10000);
        assert(res.days_remaining == 5);
        assert(res.base_quota_minor == 11'200);
        assert(res.spent_today_minor == 1'000);
        assert(res.safe_limit_today_minor == 10'200);
    }

    // 2. Адаптивное сжатие мягких целей
    {
        BudgetParams params{
            .liquid_balance_minor = 25'000,    // 250.00
            .locked_envelopes_minor = 0,
            .spent_today_minor = 0,
            .mandatory_unpaid_minor = 10'000,  // 100.00
            .hard_goals_minor = 5'000,         // 50.00
            .soft_goals_minor = 20'000,        // 200.00
            .days_remaining = 5
        };
        // Free start = 25000
        // Pool before soft = 25000 - 10000 - 5000 = 10000
        // Alpha = 10000 / 20000 = 5000 bps (50%)
        // Effective soft = 20000 * 50% = 10000
        // Net pool = 10000 - 10000 = 0
        SafeLimitResult res = calculateSafeLimit(params);
        assert(!res.is_deficit);
        assert(res.alpha_bps == 5000);
        assert(res.base_quota_minor == 0);
        assert(res.safe_limit_today_minor == 0);
    }

    // 3. Сценарий дефицита
    {
        BudgetParams params{
            .liquid_balance_minor = 10'000,    // 100.00
            .locked_envelopes_minor = 0,
            .spent_today_minor = 0,
            .mandatory_unpaid_minor = 15'000,  // 150.00 (денег не хватает на счета)
            .hard_goals_minor = 5'000,         // 50.00
            .soft_goals_minor = 5'000,
            .days_remaining = 3
        };
        // Free start = 10000
        // Pool before soft = 10000 - 15000 - 5000 = -10000
        SafeLimitResult res = calculateSafeLimit(params);
        assert(res.is_deficit);
        assert(res.alpha_bps == 0);
        assert(res.base_quota_minor == 0);
        assert(res.safe_limit_today_minor == 0);
    }

    // 4. День зарплаты (D_rem == 1): неделимый остаток копеек не теряется
    {
        BudgetParams params{
            .liquid_balance_minor = 1'005,     // 10.05
            .locked_envelopes_minor = 0,
            .spent_today_minor = 0,
            .mandatory_unpaid_minor = 0,
            .hard_goals_minor = 0,
            .soft_goals_minor = 0,
            .days_remaining = 1
        };
        SafeLimitResult res = calculateSafeLimit(params);
        assert(!res.is_deficit);
        assert(res.base_quota_minor == 1'005);
        assert(res.safe_limit_today_minor == 1'005);
    }

    // 5. Защитная обработка невалидных дней (0 или отрицательное число)
    {
        BudgetParams params{
            .liquid_balance_minor = 5'000,
            .locked_envelopes_minor = 0,
            .spent_today_minor = 0,
            .mandatory_unpaid_minor = 0,
            .hard_goals_minor = 0,
            .soft_goals_minor = 0,
            .days_remaining = 0  // Защита от деления на ноль
        };
        SafeLimitResult res = calculateSafeLimit(params);
        assert(res.days_remaining == 1);
        assert(res.base_quota_minor == 5'000);
    }

    // 6. Перерасход в течение дня (лимит уходит в контролируемый минус)
    {
        BudgetParams params{
            .liquid_balance_minor = 50'000,   // 500.00
            .locked_envelopes_minor = 0,
            .spent_today_minor = 7'000,       // Потратили 70.00
            .mandatory_unpaid_minor = 0,
            .hard_goals_minor = 0,
            .soft_goals_minor = 0,
            .days_remaining = 10
        };
        // Free start = 50000 + 7000 = 57000
        // Quota = 57000 / 10 = 5700
        // Safe limit = 5700 - 7000 = -1300 (-13.00)
        SafeLimitResult res = calculateSafeLimit(params);
        assert(res.base_quota_minor == 5'700);
        assert(res.safe_limit_today_minor == -1'300);
        assert(!res.is_deficit);
    }

    std::cout << "Budget safe limit tests passed!" << std::endl;
}


void test_money_router() {
    using namespace finance;

    // 1. Штатный сценарий (Happy path: FIXED + PERCENT)
    {
        std::vector<RoutingRule> rules = {
            {"r1", 1, RuleType::FIXED, 40'000, "env_rent"},      // 400.00
            {"r2", 2, RuleType::FIXED, 10'000, "env_bills"},     // 100.00
            {"r3", 1, RuleType::PERCENT, 1'000, "env_cushion"},  // 10%
            {"r4", 2, RuleType::PERCENT, 2'000, "env_vacation"}  // 20%
        };
        RoutingResult res = routeIncome(100'000, rules);
        assert(res.allocations.size() == 4);
        assert(res.allocations[0].envelope_id == "env_rent" && res.allocations[0].amount_minor == 40'000);
        assert(res.allocations[1].envelope_id == "env_bills" && res.allocations[1].amount_minor == 10'000);
        assert(res.allocations[2].envelope_id == "env_cushion" && res.allocations[2].amount_minor == 5'000);
        assert(res.allocations[3].envelope_id == "env_vacation" && res.allocations[3].amount_minor == 10'000);
        assert(res.remainder_minor == 35'000);
    }

    // 2. Нехватка дохода для покрытия FIXED правил
    {
        std::vector<RoutingRule> rules = {
            {"r1", 1, RuleType::FIXED, 40'000, "env_rent"},
            {"r2", 2, RuleType::FIXED, 5'000, "env_internet"},
            {"r3", 1, RuleType::PERCENT, 1'000, "env_cushion"}
        };
        RoutingResult res = routeIncome(30'000, rules);
        assert(res.allocations.size() == 1);
        assert(res.allocations[0].envelope_id == "env_rent" && res.allocations[0].amount_minor == 30'000);
        assert(res.remainder_minor == 0);
    }

    // 3. Неупорядоченный вход (проверка сортировки: сначала FIXED по priority, затем PERCENT по priority)
    {
        std::vector<RoutingRule> rules = {
            {"r_pct2", 2, RuleType::PERCENT, 1'000, "env_p2"},
            {"r_fix2", 2, RuleType::FIXED, 10'000, "env_f2"},
            {"r_pct1", 1, RuleType::PERCENT, 1'000, "env_p1"},
            {"r_fix1", 1, RuleType::FIXED, 20'000, "env_f1"}
        };
        RoutingResult res = routeIncome(100'000, rules);
        assert(res.allocations.size() == 4);
        assert(res.allocations[0].envelope_id == "env_f1" && res.allocations[0].amount_minor == 20'000);
        assert(res.allocations[1].envelope_id == "env_f2" && res.allocations[1].amount_minor == 10'000);
        assert(res.allocations[2].envelope_id == "env_p1" && res.allocations[2].amount_minor == 7'000);
        assert(res.allocations[3].envelope_id == "env_p2" && res.allocations[3].amount_minor == 7'000);
        assert(res.remainder_minor == 56'000);
    }

    // 4. Сумма процентов больше 100% (каппинг на остаток)
    {
        std::vector<RoutingRule> rules = {
            {"r1", 1, RuleType::PERCENT, 6'000, "env_p1"}, // 60%
            {"r2", 2, RuleType::PERCENT, 6'000, "env_p2"}  // 60%
        };
        RoutingResult res = routeIncome(10'000, rules);
        assert(res.allocations.size() == 2);
        assert(res.allocations[0].envelope_id == "env_p1" && res.allocations[0].amount_minor == 6'000);
        assert(res.allocations[1].envelope_id == "env_p2" && res.allocations[1].amount_minor == 4'000);
        assert(res.remainder_minor == 0);
    }

    // 5. Защитная обработка некорректных / нулевых правил
    {
        std::vector<RoutingRule> rules = {
            {"r_bad1", 1, RuleType::FIXED, -5'000, "env_bad1"},
            {"r_bad2", 2, RuleType::FIXED, 0, "env_bad2"},
            {"r_ok", 3, RuleType::FIXED, 1'000, "env_ok"}
        };
        RoutingResult res = routeIncome(10'000, rules);
        assert(res.allocations.size() == 1);
        assert(res.allocations[0].envelope_id == "env_ok" && res.allocations[0].amount_minor == 1'000);
        assert(res.remainder_minor == 9'000);
    }

    // 6. Нулевой или отрицательный доход, пустые правила
    {
        RoutingResult res1 = routeIncome(0, {});
        assert(res1.allocations.empty() && res1.remainder_minor == 0);

        RoutingResult res2 = routeIncome(-1'000, {});
        assert(res2.allocations.empty() && res2.remainder_minor == -1'000);

        std::vector<RoutingRule> rules = {{"r1", 1, RuleType::FIXED, 1'000, "env1"}};
        RoutingResult res3 = routeIncome(0, rules);
        assert(res3.allocations.empty() && res3.remainder_minor == 0);

        RoutingResult res4 = routeIncome(5'000, {});
        assert(res4.allocations.empty() && res4.remainder_minor == 5'000);
    }

    std::cout << "Money router tests passed!" << std::endl;
}


void test_reconciliation() {
    using namespace finance;

    // 1. Constexpr проверка формулы дельты
    static_assert(calculateDiscrepancy(100, 70) == 30);
    static_assert(calculateDiscrepancy(70, 100) == -30);
    static_assert(calculateDiscrepancy(100, 100) == 0);

    // 2. Балансы совпадают: транзакция НЕ создается
    {
        ReconciliationPlan plan = reconcileBalance("acc_main", 85'000, 85'000, 1700000000, "2026-11-19");
        assert(plan.result.discrepancy_minor == 0);
        assert(plan.result.generated_transaction_id.empty());
        assert(!plan.transaction.has_value());
    }

    // 3. Забытый расход (в банке меньше: комиссия, тихие списания)
    {
        // В банке 800.00, в приложении 850.00 -> расхождение -50.00 (-5000)
        ReconciliationPlan plan = reconcileBalance("acc_main", 80'000, 85'000, 1700000000, "2026-11-19");
        assert(plan.result.discrepancy_minor == -5'000);
        assert(!plan.result.generated_transaction_id.empty());
        assert(plan.transaction.has_value());

        const auto& tx = *plan.transaction;
        assert(tx.id == plan.result.generated_transaction_id);
        assert(tx.account_id == "acc_main");
        assert(tx.envelope_id.empty());
        assert(tx.amount_minor == -5'000);
        assert(tx.status == TransactionStatus::RECONCILED);
        assert(tx.timestamp_utc == 1700000000);
        assert(tx.local_date == "2026-11-19");
        assert(tx.category_id == 0);
    }

    // 4. Забытый доход (в банке больше: процент на остаток, кешбэк)
    {
        // В банке 1000.00, в приложении 950.00 -> расхождение +50.00 (+5000)
        ReconciliationPlan plan = reconcileBalance("acc_card2", 100'000, 95'000, 1700000000, "2026-11-19");
        assert(plan.result.discrepancy_minor == 5'000);
        assert(!plan.result.generated_transaction_id.empty());
        assert(plan.transaction.has_value());

        const auto& tx = *plan.transaction;
        assert(tx.id == plan.result.generated_transaction_id);
        assert(tx.account_id == "acc_card2");
        assert(tx.envelope_id.empty());
        assert(tx.amount_minor == 5'000);
        assert(tx.status == TransactionStatus::RECONCILED);
    }

    std::cout << "Reconciliation tests passed!" << std::endl;
}


int main() {
    test_rounding();
    test_currency_conversion();
    test_uuid_v7();
    test_budget_alpha();
    test_budget_safe_limit();
    test_money_router();
    test_reconciliation();

    std::cout << "\n===============================" << std::endl;
    std::cout << "All domain math tests PASSED!" << std::endl;
    std::cout << "===============================" << std::endl;
    return 0;
}
