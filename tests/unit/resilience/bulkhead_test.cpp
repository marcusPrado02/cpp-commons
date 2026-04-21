#include <bulkhead.hpp>

#include <gtest/gtest.h>

using namespace cpp_commons::resilience;

TEST(BulkheadTest, AllowsUpToLimit) {
    Bulkhead bh{2};
    EXPECT_EQ(bh.available(), 2u);
    bh.call([&] {
        bh.call([] { return 0; });  // nested — both slots used inside outer
        return 0;
    });
    EXPECT_EQ(bh.available(), 2u);
}

TEST(BulkheadTest, ThrowsWhenFull) {
    Bulkhead bh{1};
    EXPECT_THROW(bh.call([&] {
        return bh.call([] { return 0; });  // second call exceeds limit
    }),
                 BulkheadFullError);
}

TEST(BulkheadTest, AvailableRestoresAfterCall) {
    Bulkhead bh{3};
    bh.call([] { return 0; });
    EXPECT_EQ(bh.available(), 3u);
}
