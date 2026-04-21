#include <cpp_commons/errors/domain_error.hpp>
#include <cpp_commons/kernel/option.hpp>
#include <cpp_commons/kernel/result.hpp>
#include <cpp_commons/testing/matchers.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace cpp_commons::testing;
using namespace cpp_commons::kernel;
using namespace cpp_commons::errors;
using ::testing::Not;

using IntResult = Result<int, ValidationError>;
using IntOption = Option<int>;

TEST(MatchersTest, IsOkPassesForOkResult) {
    auto r = IntResult::ok(42);
    EXPECT_THAT(r, IsOk());
}

TEST(MatchersTest, IsOkFailsForErrResult) {
    auto r = IntResult::err(ValidationError{"bad"});
    EXPECT_THAT(r, Not(IsOk()));
}

TEST(MatchersTest, IsErrPassesForErrResult) {
    auto r = IntResult::err(ValidationError{"bad"});
    EXPECT_THAT(r, IsErr());
}

TEST(MatchersTest, IsErrFailsForOkResult) {
    auto r = IntResult::ok(0);
    EXPECT_THAT(r, Not(IsErr()));
}

TEST(MatchersTest, IsOkWithMatchesValue) {
    auto r = IntResult::ok(99);
    EXPECT_THAT(r, IsOkWith(99));
}

TEST(MatchersTest, IsOkWithFailsOnWrongValue) {
    auto r = IntResult::ok(1);
    EXPECT_THAT(r, Not(IsOkWith(2)));
}

TEST(MatchersTest, IsSomePassesForSomeOption) {
    auto o = IntOption::some(7);
    EXPECT_THAT(o, IsSome());
}

TEST(MatchersTest, IsSomeFailsForNoneOption) {
    auto o = IntOption::none();
    EXPECT_THAT(o, Not(IsSome()));
}

TEST(MatchersTest, IsNonePassesForNoneOption) {
    auto o = IntOption::none();
    EXPECT_THAT(o, IsNone());
}

TEST(MatchersTest, IsNoneFailsForSomeOption) {
    auto o = IntOption::some(0);
    EXPECT_THAT(o, Not(IsNone()));
}

TEST(MatchersTest, IsSomeWithMatchesValue) {
    auto o = IntOption::some(42);
    EXPECT_THAT(o, IsSomeWith(42));
}
