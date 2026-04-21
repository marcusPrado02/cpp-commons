#include <retry_policy.hpp>
#include <stdexcept>

#include <cpp_commons/kernel/deadline.hpp>

#include <gtest/gtest.h>

using namespace cpp_commons::resilience;
namespace kernel = cpp_commons::kernel;

struct TransientError : std::runtime_error {
    using std::runtime_error::runtime_error;
};
struct FatalError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

TEST(RetryPolicyTest, SucceedsOnFirstAttempt) {
    int calls = 0;
    RetryConfig cfg{.max_attempts = 3, .initial_delay = std::chrono::milliseconds{0}};
    auto result = with_retry(cfg, [&] {
        ++calls;
        return 42;
    });
    EXPECT_EQ(result, 42);
    EXPECT_EQ(calls, 1);
}

TEST(RetryPolicyTest, RetriesOnException) {
    int calls = 0;
    RetryConfig cfg{.max_attempts = 3, .initial_delay = std::chrono::milliseconds{0}};
    auto result = with_retry(cfg, [&] {
        if (++calls < 3)
            throw std::runtime_error{"transient"};
        return calls;
    });
    EXPECT_EQ(result, 3);
    EXPECT_EQ(calls, 3);
}

TEST(RetryPolicyTest, ThrowsRetryExhaustedAfterAllAttempts) {
    RetryConfig cfg{.max_attempts = 2, .initial_delay = std::chrono::milliseconds{0}};
    EXPECT_THROW(with_retry(cfg,
                            [] {
                                throw std::runtime_error{"always fails"};
                                return 0;
                            }),
                 RetryExhausted);
}

// ── retry_on<E> ──────────────────────────────────────────────────────────────

TEST(RetryPolicyTest, RetryOnMatchingExceptionRetries) {
    int calls = 0;
    RetryConfig cfg{
        .max_attempts = 3,
        .initial_delay = std::chrono::milliseconds{0},
        .should_retry = retry_on<TransientError>(),
    };
    auto result = with_retry(cfg, [&] {
        if (++calls < 3)
            throw TransientError{"transient"};
        return calls;
    });
    EXPECT_EQ(result, 3);
}

TEST(RetryPolicyTest, RetryOnNonMatchingExceptionPropagatesImmediately) {
    int calls = 0;
    RetryConfig cfg{
        .max_attempts = 5,
        .initial_delay = std::chrono::milliseconds{0},
        .should_retry = retry_on<TransientError>(),
    };
    EXPECT_THROW(with_retry(cfg,
                            [&] {
                                ++calls;
                                throw FatalError{"fatal"};
                                return 0;
                            }),
                 FatalError);
    EXPECT_EQ(calls, 1);  // no retry on unmatched exception type
}

// ── Jitter ───────────────────────────────────────────────────────────────────

TEST(RetryPolicyTest, FullJitterStaysWithinBounds) {
    // With zero initial_delay jitter has nothing to work with — use 100ms and
    // verify we don't sleep more than the delay (statistical check over N runs).
    RetryConfig cfg{
        .max_attempts = 2,
        .initial_delay = std::chrono::milliseconds{10},
        .backoff_multiplier = 1.0,
        .max_delay = std::chrono::milliseconds{10},
        .jitter = JitterStrategy::Full,
    };
    auto t0 = std::chrono::steady_clock::now();
    EXPECT_THROW(with_retry(cfg,
                            [] {
                                throw std::runtime_error{"x"};
                                return 0;
                            }),
                 RetryExhausted);
    auto elapsed = std::chrono::steady_clock::now() - t0;
    // Full jitter must sleep [0, 10ms] — with one inter-attempt sleep the
    // total elapsed must be < 50ms even with scheduler noise.
    EXPECT_LT(elapsed, std::chrono::milliseconds{50});
}

TEST(RetryPolicyTest, EqualJitterStaysWithinBounds) {
    RetryConfig cfg{
        .max_attempts = 2,
        .initial_delay = std::chrono::milliseconds{10},
        .backoff_multiplier = 1.0,
        .max_delay = std::chrono::milliseconds{10},
        .jitter = JitterStrategy::Equal,
    };
    auto t0 = std::chrono::steady_clock::now();
    EXPECT_THROW(with_retry(cfg,
                            [] {
                                throw std::runtime_error{"x"};
                                return 0;
                            }),
                 RetryExhausted);
    auto elapsed = std::chrono::steady_clock::now() - t0;
    EXPECT_LT(elapsed, std::chrono::milliseconds{50});
}

// ── Deadline ─────────────────────────────────────────────────────────────────

TEST(DeadlineTest, NeverIsNeverExpired) {
    auto d = kernel::Deadline::never();
    EXPECT_FALSE(d.is_expired());
    EXPECT_GT(d.remaining().count(), 0);
}

TEST(DeadlineTest, AlreadyExpiredDeadline) {
    auto past = std::chrono::system_clock::now() - std::chrono::seconds{1};
    kernel::Deadline d{past};
    EXPECT_TRUE(d.is_expired());
    EXPECT_EQ(d.remaining().count(), 0);
}

TEST(DeadlineTest, FutureDeadlineHasPositiveRemaining) {
    auto d = kernel::Deadline::in(std::chrono::milliseconds{500});
    EXPECT_FALSE(d.is_expired());
    EXPECT_GT(d.remaining().count(), 0);
    EXPECT_LE(d.remaining().count(), 500);
}

// ── Deadline + Retry integration ─────────────────────────────────────────────

TEST(RetryPolicyTest, DeadlineAlreadyExpiredThrowsImmediately) {
    auto past = std::chrono::system_clock::now() - std::chrono::seconds{1};
    RetryConfig cfg{
        .max_attempts = 5,
        .initial_delay = std::chrono::milliseconds{0},
        .deadline = kernel::Deadline{past},
    };
    int calls = 0;
    EXPECT_THROW(with_retry(cfg,
                            [&] {
                                ++calls;
                                return 1;
                            }),
                 DeadlineExceeded);
    EXPECT_EQ(calls, 0);  // should not have called fn at all
}

TEST(RetryPolicyTest, DeadlineNeverDoesNotAffectRetry) {
    int calls = 0;
    RetryConfig cfg{
        .max_attempts = 3,
        .initial_delay = std::chrono::milliseconds{0},
        .deadline = kernel::Deadline::never(),
    };
    auto r = with_retry(cfg, [&] {
        ++calls;
        return 42;
    });
    EXPECT_EQ(r, 42);
    EXPECT_EQ(calls, 1);
}
