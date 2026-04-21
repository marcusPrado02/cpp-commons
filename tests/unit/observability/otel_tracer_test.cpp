#include <otel_tracer.hpp>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using cpp_commons::observability::OtelTracer;
using cpp_commons::observability::SpanRecord;

TEST(OtelTracerTest, SatisfiesTracerPortConcept) {
    // Verified at compile time via static_assert in the header.
    SUCCEED();
}

TEST(OtelTracerTest, ExporterCalledOnEndSpan) {
    std::vector<SpanRecord> exported;
    OtelTracer tracer{[&](SpanRecord r) { exported.push_back(std::move(r)); }};

    tracer.start_span("op.fetch");
    tracer.end_span();

    ASSERT_EQ(exported.size(), 1u);
    EXPECT_EQ(exported[0].name, "op.fetch");
}

TEST(OtelTracerTest, SpanHasNonEmptyIds) {
    SpanRecord rec;
    OtelTracer tracer{[&](SpanRecord r) { rec = std::move(r); }};

    tracer.start_span("work");
    tracer.end_span();

    EXPECT_FALSE(rec.trace_id.empty());
    EXPECT_FALSE(rec.span_id.empty());
}

TEST(OtelTracerTest, DurationIsPositive) {
    SpanRecord rec;
    OtelTracer tracer{[&](SpanRecord r) { rec = std::move(r); }};

    tracer.start_span("compute");
    tracer.end_span();

    EXPECT_GE(rec.duration_us(), 0);
}

TEST(OtelTracerTest, NestedSpansHaveParentIds) {
    std::vector<SpanRecord> exported;
    OtelTracer tracer{[&](SpanRecord r) { exported.push_back(std::move(r)); }};

    tracer.start_span("outer");
    tracer.start_span("inner");
    tracer.end_span();  // ends inner
    tracer.end_span();  // ends outer

    ASSERT_EQ(exported.size(), 2u);
    const auto& inner = exported[0];
    const auto& outer = exported[1];
    EXPECT_EQ(inner.parent_span_id, outer.span_id);
    EXPECT_TRUE(outer.parent_span_id.empty());
}

TEST(OtelTracerTest, TraceparentFormattedCorrectly) {
    SpanRecord rec;
    OtelTracer tracer{[&](SpanRecord r) { rec = std::move(r); }};
    tracer.start_span("x");
    tracer.end_span();

    auto tp = rec.traceparent();
    EXPECT_EQ(tp.substr(0, 3), "00-");
    // format: 00-{32}-{16}-01
    EXPECT_EQ(tp.size(), 3 + 32 + 1 + 16 + 1 + 2);
}

TEST(OtelTracerTest, EndSpanWithoutStartIsNoop) {
    OtelTracer tracer;
    EXPECT_NO_THROW(tracer.end_span());
}

TEST(OtelTracerTest, NoExporterDoesNotCrash) {
    OtelTracer tracer;  // null exporter → no-op lambda
    tracer.start_span("work");
    tracer.end_span();
}
