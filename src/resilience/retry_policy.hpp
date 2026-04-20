#pragma once
#include <chrono>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <thread>

namespace cpp_commons::resilience {

struct RetryExhausted : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct RetryConfig {
    uint32_t max_attempts{3};
    std::chrono::milliseconds initial_delay{100};
    double backoff_multiplier{2.0};
    std::chrono::milliseconds max_delay{30'000};
};

// Executes fn, retrying on exception up to config.max_attempts times
// with exponential backoff. Throws RetryExhausted if all attempts fail.
template <typename Fn>
auto with_retry(const RetryConfig& cfg, Fn&& fn) -> decltype(fn()) {
    auto delay = cfg.initial_delay;
    std::exception_ptr last;
    for (uint32_t attempt = 0; attempt < cfg.max_attempts; ++attempt) {
        try {
            return fn();
        } catch (...) {
            last = std::current_exception();
            if (attempt + 1 < cfg.max_attempts) {
                std::this_thread::sleep_for(delay);
                auto next = std::chrono::milliseconds(
                    static_cast<long long>(static_cast<double>(delay.count()) * cfg.backoff_multiplier));
                delay = next < cfg.max_delay ? next : cfg.max_delay;
            }
        }
    }
    throw RetryExhausted{"all retry attempts exhausted"};
}

} // namespace cpp_commons::resilience
