#include <atomic>
#include <rate_limiter.hpp>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

using cpp_commons::resilience::RateLimiter;

TEST(RateLimiterTest, AllowsUpToMaxTokens) {
    RateLimiter limiter{5.0, 1.0};  // 5 max, refill 1/sec
    for (int i = 0; i < 5; ++i)
        EXPECT_TRUE(limiter.try_acquire()) << "Request " << i << " should be allowed";
}

TEST(RateLimiterTest, DeniesWhenExhausted) {
    RateLimiter limiter{3.0, 0.0};  // 3 max, no refill
    for (int i = 0; i < 3; ++i)
        (void)limiter.try_acquire();
    EXPECT_FALSE(limiter.try_acquire());
}

TEST(RateLimiterTest, ResetRestoresFullCapacity) {
    RateLimiter limiter{3.0, 0.0};
    for (int i = 0; i < 3; ++i)
        (void)limiter.try_acquire();
    limiter.reset();
    EXPECT_TRUE(limiter.try_acquire());
}

TEST(RateLimiterTest, AvailableTokensDecreases) {
    RateLimiter limiter{10.0, 0.0};
    double before = limiter.available_tokens();
    (void)limiter.try_acquire();
    double after = limiter.available_tokens();
    EXPECT_NEAR(after, before - 1.0, 0.01);
}

TEST(RateLimiterTest, CostGreaterThanOneConsumeMultipleTokens) {
    RateLimiter limiter{5.0, 0.0};
    EXPECT_TRUE(limiter.try_acquire(3.0));
    EXPECT_TRUE(limiter.try_acquire(2.0));
    EXPECT_FALSE(limiter.try_acquire(1.0));
}

TEST(RateLimiterTest, ThreadSafeUnderContention) {
    RateLimiter limiter{100.0, 0.0};
    std::atomic<int> allowed{0};
    std::vector<std::thread> threads;
    threads.reserve(8);
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back([&] {
            for (int j = 0; j < 20; ++j)
                if (limiter.try_acquire())
                    ++allowed;
        });
    }
    for (auto& t : threads)
        t.join();
    EXPECT_LE(allowed.load(), 100);
}
