#include "circuit_breaker.hpp"

namespace cpp_commons::resilience {

CircuitBreaker::CircuitBreaker(CircuitBreakerConfig cfg) : cfg_(cfg) {}

bool CircuitBreaker::is_open() const noexcept {
    std::lock_guard lk{mu_};
    return state_ == State::Open;
}

void CircuitBreaker::check_state() {
    std::lock_guard lk{mu_};
    if (state_ == State::Open) {
        auto elapsed = std::chrono::steady_clock::now() - opened_at_;
        if (elapsed >= cfg_.open_duration) {
            state_ = State::HalfOpen;
            success_count_ = 0;
        } else {
            throw CircuitOpenError{"circuit is open"};
        }
    }
}

void CircuitBreaker::on_success() {
    std::lock_guard lk{mu_};
    if (state_ == State::HalfOpen) {
        ++success_count_;
        if (success_count_ >= cfg_.success_threshold) {
            state_ = State::Closed;
            failure_count_ = 0;
        }
    } else {
        failure_count_ = 0;
    }
}

void CircuitBreaker::on_failure() {
    std::lock_guard lk{mu_};
    ++failure_count_;
    if (state_ == State::HalfOpen || failure_count_ >= cfg_.failure_threshold) {
        state_ = State::Open;
        opened_at_ = std::chrono::steady_clock::now();
        failure_count_ = 0;
    }
}

}  // namespace cpp_commons::resilience
