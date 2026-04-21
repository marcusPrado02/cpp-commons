/// @file clock.hpp
/// @brief Clock concept + SystemClock/FrozenClock implementations for testable time.
#pragma once
#include <chrono>
#include <concepts>

namespace cpp_commons::kernel {

using TimePoint = std::chrono::system_clock::time_point;
using Duration  = std::chrono::nanoseconds;

/// @brief Constraint: any type with a `now()` returning `TimePoint` satisfies `Clock`.
template<typename T>
concept Clock = requires(const T t) {
    { t.now() } -> std::same_as<TimePoint>;
};

/// @brief Production clock — delegates to `std::chrono::system_clock::now()`.
class SystemClock {
public:
    [[nodiscard]] TimePoint now() const noexcept {
        return std::chrono::system_clock::now();
    }
};
static_assert(Clock<SystemClock>);

/// @brief Deterministic clock for unit tests — time only advances when you call `advance()`.
class FrozenClock {
public:
    explicit FrozenClock(TimePoint t = std::chrono::system_clock::now()) : now_{t} {}

    [[nodiscard]] TimePoint now() const noexcept { return now_; }

    void set(TimePoint t) noexcept { now_ = t; }

    template<typename Rep, typename Period>
    void advance(std::chrono::duration<Rep, Period> d) {
        now_ += std::chrono::duration_cast<Duration>(d);
    }

private:
    TimePoint now_;
};
static_assert(Clock<FrozenClock>);

} // namespace cpp_commons::kernel
