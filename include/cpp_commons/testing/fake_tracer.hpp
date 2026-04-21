/// @file fake_tracer.hpp
/// @brief FakeTracer (records spans) and NoopTracer (discards) for TracerPort testing.
#pragma once
#include <string>
#include <vector>

#include <cpp_commons/kernel/ports/tracer_port.hpp>

namespace cpp_commons::testing {

/// @brief TracerPort implementation that records span names for test assertions.
class FakeTracer {
public:
    void start_span(std::string_view name) { spans_.emplace_back(name); }
    void end_span() noexcept {}

    [[nodiscard]] const std::vector<std::string>& spans() const noexcept { return spans_; }
    [[nodiscard]] bool has_span(std::string_view name) const noexcept {
        for (const auto& s : spans_)
            if (s == name)
                return true;
        return false;
    }
    void clear() noexcept { spans_.clear(); }

private:
    std::vector<std::string> spans_;
};

static_assert(kernel::TracerPort<FakeTracer>);

// Discards all spans — use when tracing is required by the port but not tested.
struct NoopTracer {
    void start_span(std::string_view) noexcept {}
    void end_span() noexcept {}
};

static_assert(kernel::TracerPort<NoopTracer>);

}  // namespace cpp_commons::testing
