#pragma once

#include <cstdint>
#include <algorithm>
#include "FinanceCoreTypes.hpp"

namespace finance {


    struct BudgetParams {
        MoneyMinor liquid_balance_minor{0};      // Текущие деньги на ликвидных счетах
        MoneyMinor locked_envelopes_minor{0};   // Деньги, запертые в целевых конвертах
        MoneyMinor spent_today_minor{0};        // Уже потрачено сегодня на повседневные расходы
        MoneyMinor mandatory_unpaid_minor{0};   // Неоплаченные счета (ЖКХ, кредиты)
        MoneyMinor hard_goals_minor{0};         // Жесткие цели (подушка, долги)
        MoneyMinor soft_goals_minor{0};         // Мягкие цели (отпуск, гаджеты)
        int32_t days_remaining{1};              // Дней до следующего дохода (включая сегодня, >= 1)
    };


    [[nodiscard]] BasisPoints calculateAlpha(
       MoneyMinor available_pool_minor,
       MoneyMinor soft_goals_minor
   ) noexcept;


        [[nodiscard]] SafeLimitResult calculateSafeLimit(
           const BudgetParams& params
       ) noexcept;
}