/// @file fake_clock.hpp
/// @brief FakeClock — deterministic Clock implementation for time-sensitive tests.
#pragma once
#include <cpp_commons/kernel/clock.hpp>

namespace cpp_commons::testing {

/// @brief Clock whose time advances only when you call `set()` or `advance()`.
class FakeClock {
public:
    explicit FakeClock(kernel::TimePoint t = std::chrono::system_clock::now()) : now_{t} {}

    [[nodiscard]] kernel::TimePoint now() const noexcept { return now_; }

    void set(kernel::TimePoint t) noexcept { now_ = t; }

    template<typename Rep, typename Period>
    void advance(std::chrono::duration<Rep, Period> d) {
        now_ += std::chrono::duration_cast<kernel::Duration>(d);
    }

private:
    kernel::TimePoint now_;
};

static_assert(kernel::Clock<FakeClock>);

} // namespace cpp_commons::testing
