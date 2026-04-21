#pragma once
#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <stdexcept>

namespace cpp_commons::resilience {

struct CircuitOpenError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct CircuitBreakerConfig {
    uint32_t failure_threshold{5};
    uint32_t success_threshold{2};
    std::chrono::seconds open_duration{30};
};

// Three-state circuit breaker (Closed → Open → Half-Open → Closed).
// Thread-safe: all state transitions protected by an internal mutex.
class CircuitBreaker {
public:
    explicit CircuitBreaker(CircuitBreakerConfig cfg = {});

    // Executes fn. Throws CircuitOpenError when open.
    // Records success/failure and drives state transitions.
    template <typename Fn>
    auto call(Fn&& fn) -> decltype(fn()) {
        check_state();
        try {
            auto result = fn();
            on_success();
            return result;
        } catch (...) {
            on_failure();
            throw;
        }
    }

    [[nodiscard]] bool is_open() const noexcept;

private:
    enum class State { Closed, Open, HalfOpen };

    void check_state();
    void on_success();
    void on_failure();

    CircuitBreakerConfig cfg_;
    mutable std::mutex mu_;
    State state_{State::Closed};
    uint32_t failure_count_{0};
    uint32_t success_count_{0};
    std::chrono::steady_clock::time_point opened_at_{};
};

}  // namespace cpp_commons::resilience
