#pragma once
#include <cstdint>
#include <cmath>
#include "FinanceCoreTypes.hpp"

namespace finance {
    inline int64_t multiplyDivide(int64_t value, int64_t numerator, int64_t denominator) noexcept {
        if (denominator == 0) return 0;
        bool negative = (value < 0) ^ (numerator < 0) ^ (denominator < 0);
        unsigned __int128 u_val = (value < 0) ? -static_cast<unsigned __int128>(value) : static_cast<unsigned __int128>(value);
        unsigned __int128 u_num = (numerator < 0) ? -static_cast<unsigned __int128>(numerator) : static_cast<unsigned __int128>(numerator);
        unsigned __int128 u_den = (denominator < 0) ? -static_cast<unsigned __int128>(denominator) : static_cast<unsigned __int128>(denominator);

        unsigned __int128 prod = u_val * u_num;
        unsigned __int128 quotient = prod / u_den;
        unsigned __int128 remainder = prod % u_den;

        if (remainder * 2 > u_den) { quotient++; }
        else if (remainder * 2 == u_den) {
            if (quotient % 2 != 0 ) { quotient++; }
        }
        int64_t res =static_cast<int64_t>(quotient);
        return negative ? -res : res;
    }

    inline MoneyMinor convertCurrency(MoneyMinor amount_minor, FxRateMicro rate_micro, BasisPoints spread_bps, bool is_buy) noexcept {

        int64_t multiplier = is_buy ? (10000 + spread_bps) : (10000 - spread_bps);
        int64_t effective_rate = multiplyDivide(rate_micro, multiplier, 10000);

        return multiplyDivide(amount_minor, effective_rate, 1'000'000);
    }
}

