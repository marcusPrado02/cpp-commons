#pragma once
#include "value_object.hpp"
#include <cpp_commons/errors/domain_error.hpp>
#include <format>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>

namespace cpp_commons::kernel {

// ISO 4217 currency code (3 uppercase letters, e.g. "USD", "EUR", "BRL").
struct CurrencyCode {
    char code[4]{};  // null-terminated 3-char string

    constexpr explicit CurrencyCode(std::string_view sv) {
        if (sv.size() != 3)
            throw std::invalid_argument{"CurrencyCode must be 3 characters"};
        code[0] = sv[0]; code[1] = sv[1]; code[2] = sv[2]; code[3] = '\0';
    }

    [[nodiscard]] std::string_view view() const noexcept { return {code, 3}; }
    bool operator==(const CurrencyCode& o) const noexcept {
        return code[0]==o.code[0] && code[1]==o.code[1] && code[2]==o.code[2];
    }
    bool operator!=(const CurrencyCode& o) const noexcept { return !(*this == o); }
};

// Immutable monetary value: integer cents + currency code.
// Arithmetic operations require matching currencies and check for overflow.
class Money : public ValueObject<Money> {
public:
    Money(int64_t cents, CurrencyCode currency)
        : cents_{cents}, currency_{currency} {}

    [[nodiscard]] int64_t     cents()    const noexcept { return cents_; }
    [[nodiscard]] CurrencyCode currency() const noexcept { return currency_; }

    [[nodiscard]] auto fields() const noexcept {
        return std::make_tuple(cents_, std::string_view{currency_.code, 3});
    }

    [[nodiscard]] std::string to_string() const {
        return std::format("{} {}.{:02}", currency_.view(),
                           cents_ / 100, std::abs(cents_ % 100));
    }

    [[nodiscard]] Money operator+(const Money& o) const {
        check_same_currency(o);
        check_no_overflow(cents_, o.cents_);
        return {cents_ + o.cents_, currency_};
    }

    [[nodiscard]] Money operator-(const Money& o) const {
        check_same_currency(o);
        check_no_overflow(cents_, -o.cents_);
        return {cents_ - o.cents_, currency_};
    }

    bool operator<(const Money& o)  const { check_same_currency(o); return cents_ < o.cents_; }
    bool operator<=(const Money& o) const { check_same_currency(o); return cents_ <= o.cents_; }
    bool operator>(const Money& o)  const { check_same_currency(o); return cents_ > o.cents_; }
    bool operator>=(const Money& o) const { check_same_currency(o); return cents_ >= o.cents_; }

private:
    int64_t     cents_;
    CurrencyCode currency_;

    void check_same_currency(const Money& o) const {
        if (currency_ != o.currency_)
            throw errors::DomainError{"Cannot operate on Money with different currencies"};
    }

    static void check_no_overflow(int64_t a, int64_t b) {
        if (b > 0 && a > std::numeric_limits<int64_t>::max() - b)
            throw errors::DomainError{"Money arithmetic overflow"};
        if (b < 0 && a < std::numeric_limits<int64_t>::min() - b)
            throw errors::DomainError{"Money arithmetic overflow"};
    }
};

} // namespace cpp_commons::kernel
