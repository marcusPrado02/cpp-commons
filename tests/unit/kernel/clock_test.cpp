#include <chrono>

#include <cpp_commons/kernel/clock.hpp>

#include <gtest/gtest.h>

using namespace cpp_commons::kernel;
using namespace std::chrono_literals;

TEST(SystemClockTest, NowIsNotEpoch) {
    SystemClock clk;
    EXPECT_GT(clk.now().time_since_epoch().count(), 0);
}

TEST(FrozenClockTest, NowIsFixed) {
    auto t = std::chrono::system_clock::now();
    FrozenClock clk{t};
    EXPECT_EQ(clk.now(), t);
    EXPECT_EQ(clk.now(), t);  // stable
}

TEST(FrozenClockTest, AdvanceMovesTime) {
    auto t = std::chrono::system_clock::now();
    FrozenClock clk{t};
    clk.advance(2h);
    EXPECT_EQ(clk.now(), t + 2h);
}

TEST(FrozenClockTest, SetOverridesTime) {
    FrozenClock clk;
    auto t = std::chrono::system_clock::now() + 10s;
    clk.set(t);
    EXPECT_EQ(clk.now(), t);
}
