#include <retry_policy.hpp>
#include <gtest/gtest.h>
#include <stdexcept>

using namespace cpp_commons::resilience;

TEST(RetryPolicyTest, SucceedsOnFirstAttempt) {
    int calls = 0;
    RetryConfig cfg{.max_attempts = 3, .initial_delay = std::chrono::milliseconds{0}};
    auto result = with_retry(cfg, [&] { ++calls; return 42; });
    EXPECT_EQ(result, 42);
    EXPECT_EQ(calls, 1);
}

TEST(RetryPolicyTest, RetriesOnException) {
    int calls = 0;
    RetryConfig cfg{.max_attempts = 3, .initial_delay = std::chrono::milliseconds{0}};
    auto result = with_retry(cfg, [&] {
        if (++calls < 3) throw std::runtime_error{"transient"};
        return calls;
    });
    EXPECT_EQ(result, 3);
    EXPECT_EQ(calls, 3);
}

TEST(RetryPolicyTest, ThrowsRetryExhaustedAfterAllAttempts) {
    RetryConfig cfg{.max_attempts = 2, .initial_delay = std::chrono::milliseconds{0}};
    EXPECT_THROW(
        with_retry(cfg, [] { throw std::runtime_error{"always fails"}; return 0; }),
        RetryExhausted);
}
