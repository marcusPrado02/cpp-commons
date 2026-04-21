#include <string>

#include <cpp_commons/kernel/result.hpp>

#include <gtest/gtest.h>

using cpp_commons::kernel::flatten;

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

TEST(ResultTest, BoolConversionOk) {
    EXPECT_TRUE(static_cast<bool>(Result<int, std::string>::ok(1)));
}
TEST(ResultTest, BoolConversionErr) {
    EXPECT_FALSE(static_cast<bool>(Result<int, std::string>::err("e")));
}

TEST(ResultTest, MapTransformsValue) {
    auto r = Result<int, std::string>::ok(5).map([](int v) { return v * v; });
    EXPECT_TRUE(r.has_value());
    EXPECT_EQ(r.value(), 25);
}

TEST(ResultTest, MapSkipsOnErr) {
    bool called = false;
    auto r = Result<int, std::string>::err("fail").map([&](int v) {
        called = true;
        return v;
    });
    EXPECT_FALSE(called);
    EXPECT_FALSE(r.has_value());
}

TEST(ResultTest, MoveSemantics) {
    auto r = Result<std::string, int>::ok("hello");
    auto moved = std::move(r).value();
    EXPECT_EQ(moved, "hello");
}

// ── map_err ──────────────────────────────────────────────────────────────────

TEST(ResultMapErrTest, TransformsErrorOnErr) {
    auto r = Result<int, std::string>::err("fail").map_err(
        [](const std::string& e) { return e.size(); });
    EXPECT_TRUE(r.is_err());
    EXPECT_EQ(r.error(), 4u);
}

TEST(ResultMapErrTest, PassesThroughOnOk) {
    auto r =
        Result<int, std::string>::ok(42).map_err([](const std::string& e) { return e.size(); });
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(r.value(), 42);
}

TEST(ResultMapErrTest, ChangesErrorType) {
    auto r = Result<int, int>::err(7).map_err([](int e) { return std::to_string(e); });
    static_assert(std::is_same_v<decltype(r.error()), std::string&>);
    EXPECT_EQ(r.error(), "7");
}

// ── flatten ───────────────────────────────────────────────────────────────────

TEST(ResultFlattenTest, OkOfOkCollapsesToOk) {
    using Inner = Result<int, std::string>;
    using Outer = Result<Inner, std::string>;
    auto nested = Outer::ok(Inner::ok(99));
    auto flat = flatten(std::move(nested));
    EXPECT_TRUE(flat.is_ok());
    EXPECT_EQ(flat.value(), 99);
}

TEST(ResultFlattenTest, OkOfErrCollapsesToErr) {
    using Inner = Result<int, std::string>;
    using Outer = Result<Inner, std::string>;
    auto nested = Outer::ok(Inner::err("inner fail"));
    auto flat = flatten(std::move(nested));
    EXPECT_TRUE(flat.is_err());
    EXPECT_EQ(flat.error(), "inner fail");
}

TEST(ResultFlattenTest, OuterErrPropagatesToErr) {
    using Inner = Result<int, std::string>;
    using Outer = Result<Inner, std::string>;
    auto nested = Outer::err("outer fail");
    auto flat = flatten(std::move(nested));
    EXPECT_TRUE(flat.is_err());
    EXPECT_EQ(flat.error(), "outer fail");
}
