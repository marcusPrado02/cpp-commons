#pragma once
#include <atomic>
#include <chrono>
#include <mutex>

#include <cpp_commons/errors/domain_error.hpp>

namespace cpp_commons::resilience {

// Thread-safe token bucket rate limiter.
// Tokens refill at `refill_rate` per second up to `max_tokens`.
class RateLimiter {
public:
    RateLimiter(double max_tokens, double refill_rate)
        : max_tokens_{max_tokens},
          refill_rate_{refill_rate},
          tokens_{max_tokens},
          last_refill_{std::chrono::steady_clock::now()} {}

    // Try to consume one token. Returns true if allowed, false if rate-limited.
    [[nodiscard]] bool try_acquire() { return try_acquire(1.0); }

    // Try to consume `cost` tokens in one call.
    [[nodiscard]] bool try_acquire(double cost) {
        std::lock_guard lock{mutex_};
        refill();
        if (tokens_ < cost)
            return false;
        tokens_ -= cost;
        return true;
    }

    // Current token count (approximate — for testing/monitoring).
    [[nodiscard]] double available_tokens() {
        std::lock_guard lock{mutex_};
        refill();
        return tokens_;
    }

    // Reset to full capacity (useful in tests).
    void reset() {
        std::lock_guard lock{mutex_};
        tokens_ = max_tokens_;
        last_refill_ = std::chrono::steady_clock::now();
    }

private:
    void refill() {
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - last_refill_).count();
        tokens_ = std::min(max_tokens_, tokens_ + elapsed * refill_rate_);
        last_refill_ = now;
    }

    double max_tokens_;
    double refill_rate_;
    double tokens_;
    std::chrono::steady_clock::time_point last_refill_;
    std::mutex mutex_;
};

}  // namespace cpp_commons::resilience
