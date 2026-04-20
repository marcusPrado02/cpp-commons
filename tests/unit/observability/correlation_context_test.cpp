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
