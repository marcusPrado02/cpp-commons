#pragma once
#include "clock.hpp"
#include <chrono>

namespace cpp_commons::kernel {

// Deadline represents an absolute point in time by which an operation must complete.
// Passed by value — cheap to copy (just a time_point).
class Deadline {
public:
    explicit Deadline(TimePoint at) noexcept : at_{at} {}

    // Convenience: deadline N milliseconds from now.
    static Deadline in(std::chrono::milliseconds ms) {
        return Deadline{std::chrono::system_clock::now() + ms};
    }

    // No deadline — operations always proceed.
    static Deadline never() {
        return Deadline{TimePoint::max()};
    }

    [[nodiscard]] bool is_expired() const noexcept {
        return std::chrono::system_clock::now() >= at_;
    }

    [[nodiscard]] std::chrono::milliseconds remaining() const noexcept {
        auto now = std::chrono::system_clock::now();
        if (now >= at_) return std::chrono::milliseconds{0};
        return std::chrono::duration_cast<std::chrono::milliseconds>(at_ - now);
    }

    [[nodiscard]] TimePoint time_point() const noexcept { return at_; }

private:
    TimePoint at_;
};

} // namespace cpp_commons::kernel
