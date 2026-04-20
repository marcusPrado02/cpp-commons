#pragma once
#include <cpp_commons/kernel/clock.hpp>

namespace cpp_commons::testing {

// FakeClock satisfies the Clock concept and allows manual time control.
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
