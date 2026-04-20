#include <circuit_breaker.hpp>
#include <gtest/gtest.h>
#include <stdexcept>

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
