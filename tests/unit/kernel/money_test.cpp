#include <limits>

#include <cpp_commons/errors/domain_error.hpp>
#include <cpp_commons/kernel/money.hpp>

#include <gtest/gtest.h>

using cpp_commons::errors::DomainError;
using cpp_commons::kernel::CurrencyCode;
using cpp_commons::kernel::Money;

static const CurrencyCode USD{"USD"};
static const CurrencyCode EUR{"EUR"};

TEST(MoneyTest, ConstructsAndAccessors) {
    Money m{1050, USD};
    EXPECT_EQ(m.cents(), 1050);
    EXPECT_EQ(m.currency(), USD);
}

TEST(MoneyTest, EqualityByValueObject) {
    EXPECT_EQ(Money(100, USD), Money(100, USD));
    EXPECT_NE(Money(100, USD), Money(200, USD));
    EXPECT_NE(Money(100, USD), Money(100, EUR));
}

TEST(MoneyTest, ToStringFormatted) {
    auto s = Money{1099, USD}.to_string();
    EXPECT_NE(s.find("10.99"), std::string::npos);
    EXPECT_NE(s.find("USD"), std::string::npos);
}

TEST(MoneyTest, AddSameCurrency) {
    Money a{500, USD}, b{300, USD};
    EXPECT_EQ((a + b).cents(), 800);
}

TEST(MoneyTest, SubtractSameCurrency) {
    Money a{500, USD}, b{300, USD};
    EXPECT_EQ((a - b).cents(), 200);
}

TEST(MoneyTest, AddDifferentCurrencyThrows) {
    EXPECT_THROW((void)(Money(100, USD) + Money(100, EUR)), DomainError);
}

TEST(MoneyTest, SubtractDifferentCurrencyThrows) {
    EXPECT_THROW((void)(Money(100, USD) - Money(100, EUR)), DomainError);
}

TEST(MoneyTest, ComparisonOperators) {
    Money a{100, USD}, b{200, USD};
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a > b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(b >= a);
}

TEST(MoneyTest, ComparisonDifferentCurrencyThrows) {
    Money a{100, USD}, b{100, EUR};
    EXPECT_THROW((void)(a < b), DomainError);
}

TEST(MoneyTest, OverflowThrows) {
    Money max{std::numeric_limits<int64_t>::max(), USD};
    EXPECT_THROW((void)(max + Money{1, USD}), DomainError);
}

TEST(CurrencyCodeTest, InvalidLengthThrows) {
    EXPECT_THROW((void)CurrencyCode{"US"}, std::invalid_argument);
    EXPECT_THROW((void)CurrencyCode{"USDD"}, std::invalid_argument);
}

TEST(CurrencyCodeTest, EqualitySameCodes) {
    EXPECT_EQ(CurrencyCode{"USD"}, CurrencyCode{"USD"});
    EXPECT_NE(CurrencyCode{"USD"}, CurrencyCode{"EUR"});
}
