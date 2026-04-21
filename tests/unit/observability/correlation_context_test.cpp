#include <correlation_context.hpp>

#include <gtest/gtest.h>

using namespace cpp_commons::observability;

TEST(CorrelationContextTest, DefaultIsEmpty) {
    // Thread might have leftover state from a previous test — use a fresh thread.
    // But within a single-threaded test binary the default is empty at start.
    EXPECT_TRUE(current_correlation().empty());
}

TEST(CorrelationContextTest, ScopeInstallsContext) {
    CorrelationContext ctx = CorrelationContext::generate();
    ctx.tenant_id = "tenant-1";

    CorrelationScope scope{ctx};
    EXPECT_EQ(current_correlation().correlation_id, ctx.correlation_id);
    EXPECT_EQ(current_correlation().tenant_id, "tenant-1");
}

TEST(CorrelationContextTest, ScopeRestoresPrevious) {
    EXPECT_TRUE(current_correlation().empty());

    {
        CorrelationScope scope{CorrelationContext::generate()};
        EXPECT_FALSE(current_correlation().empty());
    }

    EXPECT_TRUE(current_correlation().empty());
}

TEST(CorrelationContextTest, NestedScopesStack) {
    CorrelationContext outer = CorrelationContext::generate();
    outer.tenant_id = "outer";

    CorrelationContext inner = CorrelationContext::generate();
    inner.tenant_id = "inner";

    CorrelationScope scope_outer{outer};
    EXPECT_EQ(current_correlation().tenant_id, "outer");

    {
        CorrelationScope scope_inner{inner};
        EXPECT_EQ(current_correlation().tenant_id, "inner");
    }

    EXPECT_EQ(current_correlation().tenant_id, "outer");
}

TEST(CorrelationContextTest, GenerateProducesUniqueIds) {
    auto a = CorrelationContext::generate();
    auto b = CorrelationContext::generate();
    EXPECT_NE(a.correlation_id, b.correlation_id);
    EXPECT_NE(a.trace_id, b.trace_id);
}

// ── W3C Trace Context ─────────────────────────────────────────────────────────

TEST(CorrelationContextTest, TraceparentRoundTrip) {
    CorrelationContext ctx = CorrelationContext::generate();
    auto tp = ctx.traceparent();
    EXPECT_FALSE(tp.empty());

    CorrelationContext ctx2;
    EXPECT_TRUE(ctx2.set_traceparent(tp));
    EXPECT_EQ(ctx2.trace_id, ctx.trace_id);
    EXPECT_EQ(ctx2.span_id, ctx.span_id);
}

TEST(CorrelationContextTest, TraceparentFormat) {
    CorrelationContext ctx;
    ctx.trace_id = "4bf92f3577b34da6a3ce929d0e0e4736";
    ctx.span_id = "00f067aa0ba902b7";
    auto tp = ctx.traceparent();
    EXPECT_EQ(tp, "00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01");
}

TEST(CorrelationContextTest, SetTraceparentRejectsMalformed) {
    CorrelationContext ctx;
    EXPECT_FALSE(ctx.set_traceparent(""));
    EXPECT_FALSE(ctx.set_traceparent("bad"));
    EXPECT_FALSE(ctx.set_traceparent("00-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-0000000000000000-01"));
    // valid format
    EXPECT_TRUE(ctx.set_traceparent("00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01"));
}

TEST(CorrelationContextTest, GenerateProducesNonEmptyTraceAndSpanId) {
    auto ctx = CorrelationContext::generate();
    EXPECT_EQ(ctx.trace_id.size(), 32u);
    EXPECT_EQ(ctx.span_id.size(), 16u);
}
