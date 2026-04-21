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

// ── map ───────────────────────────────────────────────────────────────────────

TEST(OptionMapTest, TransformsValueOnSome) {
    auto o = Option<int>::some(5).map([](int v) { return v * v; });
    EXPECT_TRUE(o.has_value());
    EXPECT_EQ(o.value(), 25);
}

TEST(OptionMapTest, ReturnsNoneOnNone) {
    bool called = false;
    auto o = Option<int>::none().map([&](int v) { called = true; return v; });
    EXPECT_FALSE(called);
    EXPECT_FALSE(o.has_value());
}

// ── filter ────────────────────────────────────────────────────────────────────

TEST(OptionFilterTest, KeepsValueWhenPredicatePasses) {
    auto o = Option<int>::some(4).filter([](int v) { return v % 2 == 0; });
    EXPECT_TRUE(o.has_value());
    EXPECT_EQ(o.value(), 4);
}

TEST(OptionFilterTest, ReturnsNoneWhenPredicateFails) {
    auto o = Option<int>::some(3).filter([](int v) { return v % 2 == 0; });
    EXPECT_FALSE(o.has_value());
}

TEST(OptionFilterTest, NoneStaysNone) {
    auto o = Option<int>::none().filter([](int) { return true; });
    EXPECT_FALSE(o.has_value());
}

// ── value_or_else ─────────────────────────────────────────────────────────────

TEST(OptionValueOrElseTest, ReturnsValueOnSome) {
    int calls = 0;
    int v = Option<int>::some(42).value_or_else([&] { ++calls; return 0; });
    EXPECT_EQ(v, 42);
    EXPECT_EQ(calls, 0);  // factory not called
}

TEST(OptionValueOrElseTest, CallsFactoryOnNone) {
    int v = Option<int>::none().value_or_else([] { return 99; });
    EXPECT_EQ(v, 99);
}
