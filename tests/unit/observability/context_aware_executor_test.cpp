#include <context_aware_executor.hpp>
#include <correlation_context.hpp>
#include <asio/io_context.hpp>
#include <gtest/gtest.h>
#include <string>

using namespace cpp_commons::observability;

TEST(ContextAwareExecutorTest, CapturesCurrentContext) {
    CorrelationScope scope{CorrelationContext{"cid-42", "t-1", "tr-1", "req-1"}};
    asio::io_context ioc;
    auto ex = make_context_executor(ioc.get_executor());
    EXPECT_EQ(ex.context_snapshot().correlation_id, "cid-42");
}

TEST(ContextAwareExecutorTest, RestoresContextBeforeDispatch) {
    CorrelationContext outer{"outer-cid", "", "", ""};
    CorrelationContext inner{"inner-cid", "", "", ""};

    asio::io_context ioc;
    std::string seen;

    {
        CorrelationScope scope{inner};
        auto ex = make_context_executor(ioc.get_executor());

        ex.post([&seen]() {
            seen = current_correlation().correlation_id;
        }, std::allocator<void>{});
    }

    ioc.run();
    EXPECT_EQ(seen, "inner-cid");
}

TEST(ContextAwareExecutorTest, EqualityByInnerAndContext) {
    asio::io_context ioc;
    CorrelationScope s1{CorrelationContext{"cid-A", "", "", ""}};
    auto ex1 = make_context_executor(ioc.get_executor());
    auto ex2 = make_context_executor(ioc.get_executor());
    EXPECT_EQ(ex1, ex2);

    CorrelationScope s2{CorrelationContext{"cid-B", "", "", ""}};
    auto ex3 = make_context_executor(ioc.get_executor());
    EXPECT_NE(ex1, ex3);
}
