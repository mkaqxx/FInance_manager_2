#include "MoneyRouter.hpp"

#include "utils/Int128Math.hpp"


namespace finance {
    RoutingResult routeIncome(MoneyMinor income_minor, std::vector<RoutingRule> rules) noexcept {
        if (income_minor <= 0) return RoutingResult({}, income_minor);
        if (rules.empty()) return RoutingResult({}, income_minor);
        std::sort(rules.begin(), rules.end(), [](const RoutingRule& a, const RoutingRule& b) {
            return  a.priority < b.priority;
        });
        std::vector<EnvelopeAllocation> allocations{};
        MoneyMinor remaining_minor = income_minor;
        for (const auto& rule : rules) {
            if (rule.type == RuleType::FIXED) {
                if (rule.value <= 0) continue;
                MoneyMinor allocated =std::min(rule.value, remaining_minor);
                remaining_minor -= allocated;
                allocations.emplace_back(rule.target_envelope_id, rule.id, allocated);
                if (remaining_minor == 0) break;
            }
        }
        if (remaining_minor != 0) {
            MoneyMinor pool_for_percent = remaining_minor;
            for (const auto& rule : rules) {
                if (rule.type == RuleType::PERCENT) {
                    if (rule.value <= 0) continue;
                    MoneyMinor allocated = std::min(multiplyDivide(remaining_minor, rule.value, 10000), pool_for_percent);
                    pool_for_percent -= allocated;
                    allocations.emplace_back(rule.target_envelope_id, rule.id, allocated);
                    if (pool_for_percent == 0) break;
                }
            }
            remaining_minor = pool_for_percent;
        }
        return RoutingResult(allocations, remaining_minor);
    }
}
