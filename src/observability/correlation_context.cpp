#include "correlation_context.hpp"

#include <format>

namespace cpp_commons::observability {

namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
thread_local CorrelationContext tl_current{};

bool is_hex_string(std::string_view s, std::size_t expected_len) noexcept {
    if (s.size() != expected_len)
        return false;
    for (char c : s)
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
            return false;
    return true;
}
}  // namespace

const CorrelationContext& current_correlation() noexcept {
    return tl_current;
}

// W3C traceparent: "00-{32hex}-{16hex}-{8hex}"
bool CorrelationContext::set_traceparent(std::string_view tp) noexcept {
    // Must have exactly 3 dashes at positions 2, 35, 52
    if (tp.size() < 55)
        return false;
    if (tp[2] != '-' || tp[35] != '-' || tp[52] != '-')
        return false;

    auto version = tp.substr(0, 2);
    auto tid = tp.substr(3, 32);
    auto sid = tp.substr(36, 16);
    // flags at tp[53..54] — we accept any value

    if (!is_hex_string(version, 2))
        return false;
    if (!is_hex_string(tid, 32))
        return false;
    if (!is_hex_string(sid, 16))
        return false;

    trace_id = std::string{tid};
    span_id = std::string{sid};
    return true;
}

std::string CorrelationContext::traceparent() const {
    if (trace_id.empty() || span_id.empty())
        return {};
    return std::format("00-{}-{}-01", trace_id, span_id);
}

CorrelationScope::CorrelationScope(CorrelationContext ctx) : previous_(tl_current) {
    tl_current = std::move(ctx);
}

CorrelationScope::~CorrelationScope() {
    tl_current = std::move(previous_);
}

}  // namespace cpp_commons::observability
