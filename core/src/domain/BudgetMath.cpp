#include "BudgetMath.hpp"
#include "utils/Int128Math.hpp"


namespace  finance {
    BasisPoints calculateAlpha(MoneyMinor available_pool_minor, MoneyMinor soft_goals_minor) noexcept {
        if (soft_goals_minor<=0) return 10000; //100%
        if (available_pool_minor<=0) return 0; //0%
        if (available_pool_minor>=soft_goals_minor) return 10000;
        return static_cast<BasisPoints>( multiplyDivide(available_pool_minor, 10000, soft_goals_minor));
    }


    SafeLimitResult calculateSafeLimit( const BudgetParams& params) noexcept {
        int32_t days_rem = std::max(1, params.days_remaining);
        MoneyMinor available_pool_today =params.liquid_balance_minor - params.locked_envelopes_minor + params.spent_today_minor;
        MoneyMinor pool_before_soft_goal = available_pool_today - params.mandatory_unpaid_minor - params.hard_goals_minor;
        auto alpha = calculateAlpha(pool_before_soft_goal, params.soft_goals_minor);
        MoneyMinor soft_effective = multiplyDivide(params.soft_goals_minor, alpha, 10000);
        MoneyMinor safe_limit = pool_before_soft_goal - soft_effective;
        bool is_deficit = false;
        MoneyMinor base_quota_minor = 0;
        if (safe_limit < 0) {
            is_deficit = true;
            base_quota_minor = 0;
        }
        else if (days_rem>1) base_quota_minor= safe_limit/days_rem;
        else if (days_rem==1) base_quota_minor = safe_limit;
        MoneyMinor safe_limit_today = base_quota_minor - params.spent_today_minor;
        return SafeLimitResult(safe_limit_today, base_quota_minor, params.spent_today_minor,  days_rem, alpha, is_deficit);
    }
}
