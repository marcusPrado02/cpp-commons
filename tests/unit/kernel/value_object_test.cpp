#include <string>
#include <tuple>

#include <cpp_commons/kernel/value_object.hpp>

#include <gtest/gtest.h>

using cpp_commons::kernel::ValueObject;

struct Money : ValueObject<Money> {
    int64_t cents{};
    std::string currency;
    auto fields() const { return std::tie(cents, currency); }
    static Money of(int64_t c, std::string cur) {
        Money m;
        m.cents = c;
        m.currency = std::move(cur);
        return m;
    }
};

TEST(ValueObjectTest, EqualWhenFieldsMatch) {
    auto a = Money::of(100, "BRL");
    auto b = Money::of(100, "BRL");
    EXPECT_EQ(a, b);
}

TEST(ValueObjectTest, NotEqualWhenAmountDiffers) {
    auto a = Money::of(100, "BRL");
    auto b = Money::of(200, "BRL");
    EXPECT_NE(a, b);
}

TEST(ValueObjectTest, NotEqualWhenCurrencyDiffers) {
    auto a = Money::of(100, "BRL");
    auto b = Money::of(100, "USD");
    EXPECT_NE(a, b);
}
