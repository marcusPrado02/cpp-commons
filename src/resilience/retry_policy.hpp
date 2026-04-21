#pragma once
#include <chrono>
#include <cstdint>
#include <exception>
#include <functional>
#include <optional>
#include <random>
#include <stdexcept>
#include <thread>

#include <cpp_commons/kernel/deadline.hpp>

namespace cpp_commons::resilience {

struct RetryExhausted : std::runtime_error {
    using std::runtime_error::runtime_error;
};

enum class JitterStrategy {
    None,   // deterministic exponential backoff
    Full,   // uniform in [0, computed_delay]
    Equal,  // computed_delay/2 + uniform in [0, computed_delay/2]
};

struct RetryConfig {
    uint32_t max_attempts{3};
    std::chrono::milliseconds initial_delay{100};
    double backoff_multiplier{2.0};
    std::chrono::milliseconds max_delay{30'000};
    JitterStrategy jitter{JitterStrategy::None};
    // Optional predicate — if set, only retry when it returns true for the
    // current exception. Non-matching exceptions propagate immediately.
    std::function<bool(std::exception_ptr)> should_retry{};
    // Optional deadline — retries stop if deadline is exceeded.
    std::optional<kernel::Deadline> deadline{};
};

// Build a should_retry predicate that matches exceptions of type E.
template <typename E>
std::function<bool(std::exception_ptr)> retry_on() {
    return [](std::exception_ptr p) -> bool {
        try {
            std::rethrow_exception(p);
        } catch (const E&) {
            return true;
        } catch (...) {
            return false;
        }
    };
}

namespace detail {

inline std::chrono::milliseconds apply_jitter(std::chrono::milliseconds base,
                                              JitterStrategy strategy) {
    if (strategy == JitterStrategy::None || base.count() == 0)
        return base;
    thread_local std::mt19937_64 rng{std::random_device{}()};
    auto ms = base.count();
    if (strategy == JitterStrategy::Full) {
        std::uniform_int_distribution<long long> dist{0, ms};
        return std::chrono::milliseconds{dist(rng)};
    }
    // Equal: half deterministic + half random
    std::uniform_int_distribution<long long> dist{0, ms / 2};
    return std::chrono::milliseconds{ms / 2 + dist(rng)};
}

}  // namespace detail

// Executes fn, retrying on exception up to config.max_attempts times
// with exponential backoff. Throws RetryExhausted if all attempts fail.
// If cfg.should_retry is set, non-matching exceptions propagate immediately.
struct DeadlineExceeded : std::runtime_error {
    using std::runtime_error::runtime_error;
};

template <typename Fn>
auto with_retry(const RetryConfig& cfg, Fn&& fn) -> decltype(fn()) {
    auto delay = cfg.initial_delay;
    for (uint32_t attempt = 0; attempt < cfg.max_attempts; ++attempt) {
        if (cfg.deadline && cfg.deadline->is_expired())
            throw DeadlineExceeded{"deadline exceeded before attempt " + std::to_string(attempt)};
        try {
            return fn();
        } catch (...) {
            auto ep = std::current_exception();
            if (cfg.should_retry && !cfg.should_retry(ep))
                std::rethrow_exception(ep);
            if (attempt + 1 < cfg.max_attempts) {
                if (cfg.deadline && cfg.deadline->is_expired())
                    throw DeadlineExceeded{"deadline exceeded after attempt " +
                                           std::to_string(attempt)};
                std::this_thread::sleep_for(detail::apply_jitter(delay, cfg.jitter));
                auto next = std::chrono::milliseconds(static_cast<long long>(
                    static_cast<double>(delay.count()) * cfg.backoff_multiplier));
                delay = next < cfg.max_delay ? next : cfg.max_delay;
            }
        }
    }
    throw RetryExhausted{"all retry attempts exhausted"};
}

}  // namespace cpp_commons::resilience
