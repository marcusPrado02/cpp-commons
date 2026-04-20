#include <cpp_commons/kernel/option.hpp>
#include <gtest/gtest.h>
#include <string>

using cpp_commons::kernel::Option;

TEST(OptionTest, SomeHasValue) {
    auto o = Option<int>::some(7);
    EXPECT_TRUE(o.has_value());
    EXPECT_FALSE(o.is_none());
    EXPECT_EQ(o.value(), 7);
}

TEST(OptionTest, NoneHasNoValue) {
    auto o = Option<int>::none();
    EXPECT_FALSE(o.has_value());
    EXPECT_TRUE(o.is_none());
}

TEST(OptionTest, ValueOrReturnsDefault) {
    EXPECT_EQ(Option<int>::none().value_or(99),  99);
    EXPECT_EQ(Option<int>::some(1).value_or(99),  1);
}

TEST(OptionTest, BoolConversion) {
    EXPECT_TRUE(static_cast<bool>(Option<int>::some(1)));
    EXPECT_FALSE(static_cast<bool>(Option<int>::none()));
}
