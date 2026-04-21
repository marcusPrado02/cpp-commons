#pragma once
#include <optional>
#include <string>
#include <string_view>

#include <cpp_commons/kernel/identity.hpp>

namespace cpp_commons::observability {

struct CorrelationContext {
    std::string correlation_id{};
    std::string tenant_id{};
    std::string trace_id{};  // 32-char hex (W3C trace-id)
    std::string span_id{};   // 16-char hex (W3C parent-id)
    std::string request_id{};
    std::string tracestate{};  // W3C tracestate header value (opaque)

    [[nodiscard]] static CorrelationContext generate() {
        CorrelationContext ctx;
        ctx.correlation_id = kernel::UUID::generate().to_string();
        ctx.trace_id = kernel::UUID::generate().to_string_no_dashes();
        ctx.span_id = ctx.trace_id.substr(0, 16);
        ctx.request_id = kernel::UUID::generate().to_string();
        return ctx;
    }

    // Parse a W3C traceparent header into this context.
    // Format: "00-{32hex}-{16hex}-{8hex}"
    // Returns false if the header is malformed; context is unchanged.
    [[nodiscard]] bool set_traceparent(std::string_view traceparent) noexcept;

    // Produce the W3C traceparent header value ("00-{trace_id}-{span_id}-01").
    [[nodiscard]] std::string traceparent() const;

    [[nodiscard]] bool empty() const noexcept { return correlation_id.empty(); }
};

// Returns the context installed by the innermost active CorrelationScope.
[[nodiscard]] const CorrelationContext& current_correlation() noexcept;

// RAII guard: installs ctx as current, restores previous on destruction.
class CorrelationScope {
public:
    explicit CorrelationScope(CorrelationContext ctx);
    ~CorrelationScope();
    CorrelationScope(const CorrelationScope&) = delete;
    CorrelationScope& operator=(const CorrelationScope&) = delete;
    CorrelationScope(CorrelationScope&&) = delete;
    CorrelationScope& operator=(CorrelationScope&&) = delete;

private:
    CorrelationContext previous_;
};

}  // namespace cpp_commons::observability
