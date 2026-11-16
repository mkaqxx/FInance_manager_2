#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <optional>
namespace finance {

    using MoneyMinor = int64_t;
    using FxRateMicro = int64_t;
    using BasisPoints = int32_t;

    enum class Currency : uint8_t {
        BYN, USD, EUR, RUB
    };

    enum class TransactionStatus : uint8_t {
        PENDING_AUTH, CLEARED, RECONCILED
    };

    enum class PriorityTier : uint8_t {
        HARD = 1, SOFT = 2
    };

    enum class RuleType : uint8_t {
        FIXED, PERCENT
    };

    enum class GoalKind : uint8_t {
        MANDATORY_BILL, HARD_GOAL, SOFT_GOAL, INCOME_ANCHOR
    };

    //Currency utils
    constexpr std::string_view currencyToString(Currency currency) noexcept {
        switch (currency) {
            case Currency::BYN: return "BYN";
            case Currency::USD: return "USD";
            case Currency::EUR: return "EUR";
            case Currency::RUB: return "RUB";
            default: return "UNKNOWN";
        }
    }

    constexpr std::optional<Currency> currencyFromString(std::string_view currency) noexcept {
        if (currency == "BYN") { return Currency::BYN; }
        if (currency == "USD") { return Currency::USD; }
        if (currency == "EUR") { return Currency::EUR; }
        if (currency == "RUB") { return Currency::RUB; }
        return std::nullopt;
    }

    //transactions status utils
    constexpr std::string_view transactionStatusToString(TransactionStatus status) noexcept {
        switch (status) {
            case TransactionStatus::PENDING_AUTH: return "PENDING_AUTH";
            case TransactionStatus::CLEARED: return "CLEARED";
            case TransactionStatus::RECONCILED: return "RECONCILED";
            default: return "UNKNOWN";
        }
    }

    constexpr std::optional<TransactionStatus> transactionStatusFromString(std::string_view status) noexcept {
        if (status == "PENDING_AUTH") {return TransactionStatus::PENDING_AUTH;}
        if (status == "CLEARED") {return TransactionStatus::CLEARED;}
        if (status == "RECONCILED") {return TransactionStatus::RECONCILED;}
        return std::nullopt;
    }

    //PriorityTier utils
    constexpr std::string_view priorityTierToString(PriorityTier priority) noexcept {
        switch (priority) {
            case PriorityTier::HARD: return "HARD";
            case PriorityTier::SOFT: return "SOFT";
            default: return "UNKNOWN";
        }
    }

    constexpr std::optional<PriorityTier> priorityTierFromString(std::string_view priority) noexcept {
        if (priority == "HARD") { return PriorityTier::HARD; }
        if (priority == "SOFT") { return PriorityTier::SOFT; }
        return std::nullopt;
    }

    //RuleType utils
    constexpr std::string_view ruleTypeToString(RuleType type) noexcept {
        switch (type) {
            case RuleType::FIXED: return "FIXED";
            case RuleType::PERCENT: return "PERCENT";
            default: return "UNKNOWN";
        }
    }

    constexpr std::optional<RuleType> ruleTypeFromString(std::string_view type) noexcept {
        if (type == "FIXED") { return RuleType::FIXED; }
        if (type == "PERCENT") { return RuleType::PERCENT; }
        return std::nullopt;
    }

    //GoalKind utils

    constexpr std::string_view goalKindToString(GoalKind kind) noexcept {
        switch (kind) {
            case GoalKind::MANDATORY_BILL: return "MANDATORY_BILL";
            case GoalKind::HARD_GOAL: return "HARD_GOAL";
            case GoalKind::SOFT_GOAL: return "SOFT_GOAL";
            case GoalKind::INCOME_ANCHOR: return "INCOME_ANCHOR";
            default: return "UNKNOWN";
        }
    }

    constexpr std::optional<GoalKind> goalKindFromString(std::string_view kind) noexcept {
        if (kind == "MANDATORY_BILL") { return GoalKind::MANDATORY_BILL; }
        if (kind == "HARD_GOAL") { return GoalKind::HARD_GOAL; }
        if (kind == "SOFT_GOAL") { return GoalKind::SOFT_GOAL; }
        if (kind == "INCOME_ANCHOR") { return GoalKind::INCOME_ANCHOR; }
        return std::nullopt;
    }


    //DTO structs

    struct AccountDto {
        std::string id;
        std::string name;
        Currency currency;
        bool is_liquid;
        std::string card_last4;
        MoneyMinor calculated_balance;
    };

    struct EnvelopeDto {
        std::string id;
        std::string name;
        std::string linked_account_id;
        MoneyMinor target_minor;
        PriorityTier priority;
        MoneyMinor current_balance;
    };

    struct TransactionDto {
        std::string id;
        std::string account_id;
        std::string envelope_id;
        int64_t timestamp_utc;
        std::string local_date;
        MoneyMinor amount_minor;
        std::string payee_raw;
        std::string payee_normalized;
        uint32_t category_id;
        TransactionStatus status;
    };

    struct SafeLimitResult {
        MoneyMinor safe_limit_today_minor;
        MoneyMinor base_quota_minor;
        MoneyMinor spent_today_minor;
        int32_t days_remaining;
        BasisPoints alpha_bps;
        bool is_deficit;
    };

    struct ReconciliationResult {
        MoneyMinor discrepancy_minor;
        std::string generated_transaction_id;
    };

}//namespace finance


