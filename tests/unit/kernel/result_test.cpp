#include <cpp_commons/kernel/result.hpp>
#include <gtest/gtest.h>
#include <string>

using cpp_commons::kernel::Result;

TEST(ResultTest, OkHoldsValue) {
    auto r = Result<int, std::string>::ok(42);
    EXPECT_TRUE(r.is_ok());
    EXPECT_FALSE(r.is_err());
    EXPECT_EQ(r.value(), 42);
}

TEST(ResultTest, ErrHoldsError) {
    auto r = Result<int, std::string>::err("oops");
    EXPECT_FALSE(r.is_ok());
    EXPECT_TRUE(r.is_err());
    EXPECT_EQ(r.error(), "oops");
}

TEST(ResultTest, BoolConversionOk)  { EXPECT_TRUE(static_cast<bool>(Result<int, std::string>::ok(1)));   }
TEST(ResultTest, BoolConversionErr) { EXPECT_FALSE(static_cast<bool>(Result<int, std::string>::err("e"))); }

TEST(ResultTest, MapTransformsValue) {
    auto r = Result<int, std::string>::ok(5).map([](int v) { return v * v; });
    EXPECT_TRUE(r.has_value());
    EXPECT_EQ(r.value(), 25);
}

TEST(ResultTest, MapSkipsOnErr) {
    bool called = false;
    auto r = Result<int, std::string>::err("fail").map([&](int v) { called = true; return v; });
    EXPECT_FALSE(called);
    EXPECT_FALSE(r.has_value());
}

TEST(ResultTest, MoveSemantics) {
    auto r = Result<std::string, int>::ok("hello");
    auto moved = std::move(r).value();
    EXPECT_EQ(moved, "hello");
}
