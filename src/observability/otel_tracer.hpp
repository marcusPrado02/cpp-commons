#pragma once
#include <cpp_commons/kernel/ports/tracer_port.hpp>
#include "correlation_context.hpp"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <format>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace cpp_commons::observability {

// Completed span record — W3C Trace Context compatible.
struct SpanRecord {
    std::string trace_id;        // 16-byte hex (32 chars)
    std::string span_id;         // 8-byte hex (16 chars)
    std::string parent_span_id;  // empty if root span
    std::string name;
    std::chrono::steady_clock::time_point start;
    std::chrono::steady_clock::time_point end;

    [[nodiscard]] long long duration_us() const {
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }

    // W3C traceparent: "00-{trace_id}-{span_id}-01"
    [[nodiscard]] std::string traceparent() const {
        return std::format("00-{}-{}-01", trace_id, span_id);
    }
};

// Lightweight OpenTelemetry-compatible tracer that satisfies TracerPort.
// Spans are collected in memory and exported via a pluggable exporter function.
// No external SDK dependency — designed to be swapped for otel-cpp later.
class OtelTracer {
public:
    using Exporter = std::function<void(SpanRecord)>;

    // Exporter is called once per completed span, from end_span().
    explicit OtelTracer(Exporter exporter = nullptr)
        : exporter_(exporter ? std::move(exporter) : [](SpanRecord){}) {}

    void start_span(std::string_view name) {
        std::lock_guard lock{mutex_};
        ActiveSpan span;
        span.record.name   = std::string{name};
        span.record.start  = std::chrono::steady_clock::now();

        const auto& ctx = current_correlation();
        if (!ctx.empty()) {
            span.record.trace_id = ctx.correlation_id;
        } else {
            span.record.trace_id = generate_id(16);
        }
        span.record.span_id = generate_id(8);
        if (!stack_.empty()) {
            span.record.parent_span_id = stack_.back().record.span_id;
        }
        stack_.push_back(std::move(span));
    }

    void end_span() {
        std::lock_guard lock{mutex_};
        if (stack_.empty()) return;
        auto& top = stack_.back();
        top.record.end = std::chrono::steady_clock::now();
        exporter_(top.record);
        stack_.pop_back();
    }

private:
    struct ActiveSpan { SpanRecord record; };

    static std::string generate_id(std::size_t bytes) {
        // Deterministic for testing: use a simple counter.
        // In production, replace with SecureRandom::generate_bytes.
        static std::atomic<uint64_t> counter{0x1a2b3c4d5e6f7890ULL};
        std::string id;
        for (std::size_t i = 0; i < bytes; ++i) {
            uint8_t byte = static_cast<uint8_t>(counter.fetch_add(1, std::memory_order_relaxed));
            id += std::format("{:02x}", byte);
        }
        return id;
    }

    Exporter exporter_;
    std::vector<ActiveSpan> stack_;  // call stack of active spans
    std::mutex mutex_;
};

static_assert(kernel::TracerPort<OtelTracer>);

} // namespace cpp_commons::observability
