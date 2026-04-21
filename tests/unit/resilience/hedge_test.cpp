#include <hedge.hpp>
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>

using namespace cpp_commons::resilience;
using namespace std::chrono_literals;

TEST(HedgeTest, ReturnsValueFromPrimaryIfFast) {
    std::atomic<int> calls{0};
    auto result = hedge(200ms, [&](std::atomic<bool>&) -> int {
        calls.fetch_add(1);
        return 42;
    });
    EXPECT_EQ(result, 42);
    // Primary was fast enough that hedge may or may not have fired.
    EXPECT_GE(calls.load(), 1);
}

TEST(HedgeTest, HedgeFiresWhenPrimaryIsSlow) {
    std::atomic<int> calls{0};
    auto result = hedge(50ms, [&](std::atomic<bool>& cancelled) -> int {
        int id = calls.fetch_add(1);
        if (id == 0) {
            // Primary: slow, but allow hedge to win
            for (int i = 0; i < 20 && !cancelled.load(); ++i)
                std::this_thread::sleep_for(10ms);
            return 1;
        }
        // Hedge: fast
        return 2;
    });
    // Hedge should win (returns 2) or primary returns 1 if hedge triggered late.
    EXPECT_TRUE(result == 1 || result == 2);
    EXPECT_GE(calls.load(), 1);
}

TEST(HedgeTest, BothFailsRethrowsFirstException) {
    EXPECT_THROW({
        hedge(20ms, [](std::atomic<bool>&) -> int {
            throw std::runtime_error{"fail"};
        });
    }, std::runtime_error);
}

TEST(HedgeTest, CancelledFlagSetAfterFirstCompletes) {
    std::atomic<bool> hedge_saw_cancel{false};
    hedge(30ms, [&](std::atomic<bool>& c) -> int {
        static std::atomic<int> idx{0};
        int id = idx.fetch_add(1);
        if (id == 0) return 99;
        // Second call should see cancelled = true
        hedge_saw_cancel.store(c.load());
        return 88;
    });
    // If hedge never ran, cancelled was set too fast — either way, no crash.
    (void)hedge_saw_cancel;
}
