#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include "FinanceCoreTypes.hpp"

namespace finance {

    struct RoutingRule {
        std::string id;
        int32_t priority{0};            // Чем меньше число, тем выше приоритет
        RuleType type{RuleType::FIXED}; // FIXED или PERCENT
        int64_t value{0};               // FIXED: копейки (MoneyMinor)
                                        // PERCENT: базисные пункты (BasisPoints, 10000 = 100%)
        std::string target_envelope_id;
    };

    struct EnvelopeAllocation {
        std::string envelope_id;
        std::string rule_id;
        MoneyMinor amount_minor{0};
    };

    struct RoutingResult {
        std::vector<EnvelopeAllocation> allocations;
        MoneyMinor remainder_minor{0}; // Нераспределенный свободный остаток на карте
    };

    [[nodiscard]] RoutingResult routeIncome(
        MoneyMinor income_minor,
        std::vector<RoutingRule> rules
    ) noexcept;

} // namespace finance