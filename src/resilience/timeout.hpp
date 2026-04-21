#pragma once
#include <chrono>
#include <future>
#include <stdexcept>

namespace cpp_commons::resilience {

struct TimeoutError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// Runs fn on a detached std::async thread; throws TimeoutError if the
// result is not ready within the given duration.
template <typename Fn>
auto with_timeout(std::chrono::milliseconds limit, Fn&& fn) -> decltype(fn()) {
    auto fut = std::async(std::launch::async, std::forward<Fn>(fn));
    if (fut.wait_for(limit) == std::future_status::timeout) {
        throw TimeoutError{"operation timed out"};
    }
    return fut.get();
}

}  // namespace cpp_commons::resilience
