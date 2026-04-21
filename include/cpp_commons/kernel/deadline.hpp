/// @file deadline.hpp
/// @brief Deadline — absolute time point by which an operation must complete.
#pragma once
#include "clock.hpp"

#include <chrono>

namespace cpp_commons::kernel {

/// @brief Deadline wraps a `TimePoint` and exposes `is_expired()` / `remaining()`.
///
/// Passed by value (16 bytes). Use `Deadline::in(ms)` for relative construction,
/// `Deadline::never()` to express an unconditional permission.
class Deadline {
public:
    explicit Deadline(TimePoint at) noexcept : at_{at} {}

    // Convenience: deadline N milliseconds from now.
    static Deadline in(std::chrono::milliseconds ms) {
        return Deadline{std::chrono::system_clock::now() + ms};
    }

    // No deadline — operations always proceed.
    static Deadline never() { return Deadline{TimePoint::max()}; }

    [[nodiscard]] bool is_expired() const noexcept {
        return std::chrono::system_clock::now() >= at_;
    }

    [[nodiscard]] std::chrono::milliseconds remaining() const noexcept {
        auto now = std::chrono::system_clock::now();
        if (now >= at_)
            return std::chrono::milliseconds{0};
        return std::chrono::duration_cast<std::chrono::milliseconds>(at_ - now);
    }

    [[nodiscard]] TimePoint time_point() const noexcept { return at_; }

private:
    TimePoint at_;
};

}  // namespace cpp_commons::kernel
