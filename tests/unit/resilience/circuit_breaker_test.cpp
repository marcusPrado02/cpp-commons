#include <circuit_breaker.hpp>
#include <gtest/gtest.h>
#include <atomic>
#include <stdexcept>
#include <thread>
#include <vector>

using namespace cpp_commons::resilience;

TEST(CircuitBreakerTest, ClosedStatePassesThrough) {
    CircuitBreaker cb{CircuitBreakerConfig{.failure_threshold = 3}};
    EXPECT_EQ(cb.call([] { return 7; }), 7);
    EXPECT_FALSE(cb.is_open());
}

TEST(CircuitBreakerTest, OpensAfterFailureThreshold) {
    CircuitBreaker cb{CircuitBreakerConfig{.failure_threshold = 2, .open_duration = std::chrono::seconds{60}}};
    for (int i = 0; i < 2; ++i) {
        EXPECT_THROW(cb.call([]() -> int { throw std::runtime_error{"fail"}; }), std::runtime_error);
    }
    EXPECT_TRUE(cb.is_open());
}

TEST(CircuitBreakerTest, ThrowsCircuitOpenErrorWhenOpen) {
    CircuitBreaker cb{CircuitBreakerConfig{.failure_threshold = 1, .open_duration = std::chrono::seconds{60}}};
    EXPECT_THROW(cb.call([]() -> int { throw std::runtime_error{"fail"}; }), std::runtime_error);
    EXPECT_THROW(cb.call([] { return 1; }), CircuitOpenError);
}

// ── Concurrency tests ────────────────────────────────────────────────────────

TEST(CircuitBreakerConcurrencyTest, NoDataRaceUnderConcurrentSuccess) {
    constexpr int kThreads = 8;
    constexpr int kCallsPerThread = 100;
    CircuitBreaker cb{CircuitBreakerConfig{.failure_threshold = 1000}};

    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&cb] {
            for (int j = 0; j < kCallsPerThread; ++j)
                (void)cb.call([] { return 1; });
        });
    }
    for (auto& t : threads) t.join();

    EXPECT_FALSE(cb.is_open());
}

TEST(CircuitBreakerConcurrencyTest, OpensEventuallyUnderConcurrentFailures) {
    constexpr int kThreads = 8;
    constexpr uint32_t kThreshold = 10;
    CircuitBreaker cb{CircuitBreakerConfig{
        .failure_threshold = kThreshold,
        .open_duration     = std::chrono::seconds{60}}};

    std::atomic<int> failures{0};
    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&cb, &failures] {
            for (int j = 0; j < 5; ++j) {
                try {
                    (void)cb.call([]() -> int { throw std::runtime_error{"fail"}; });
                } catch (const CircuitOpenError&) {
                    // open — expected after threshold
                } catch (...) {
                    failures.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }
    for (auto& t : threads) t.join();

    // After 8*5=40 attempts (>threshold=10), circuit must be open
    EXPECT_TRUE(cb.is_open());
}

TEST(CircuitBreakerConcurrencyTest, StateIsConsistentAcrossThreads) {
    constexpr int kThreads = 8;
    CircuitBreaker cb{CircuitBreakerConfig{
        .failure_threshold = 3,
        .open_duration     = std::chrono::seconds{60}}};

    // Trip the breaker single-threaded first
    for (int i = 0; i < 3; ++i) {
        try {
            (void)cb.call([]() -> int { throw std::runtime_error{"fail"}; });
        } catch (...) {}
    }
    ASSERT_TRUE(cb.is_open());

    // All concurrent reads must agree it's open
    std::atomic<int> open_count{0};
    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&cb, &open_count] {
            if (cb.is_open())
                open_count.fetch_add(1, std::memory_order_relaxed);
        });
    }
    for (auto& t : threads) t.join();
    EXPECT_EQ(open_count.load(), kThreads);
}
