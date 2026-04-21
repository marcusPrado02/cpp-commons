#include <command_bus.hpp>
#include <query_bus.hpp>
#include <tracing_middleware.hpp>

#include <cpp_commons/testing/fake_tracer.hpp>

#include <gtest/gtest.h>

using namespace cpp_commons::application;
namespace ct = cpp_commons::testing;

struct PlaceOrder {
    int product_id;
};
struct GetOrderCount {};

TEST(TracingCommandHandlerTest, StartsAndEndSpanAroundCommand) {
    ct::FakeTracer tracer;
    CommandBus bus;
    bool handled = false;

    bus.register_handler<PlaceOrder>(tracing_command_handler<ct::FakeTracer, PlaceOrder>(
        tracer, [&](const PlaceOrder&) { handled = true; }));

    bus.send(PlaceOrder{1});

    EXPECT_TRUE(handled);
    EXPECT_EQ(tracer.spans().size(), 1u);
}

TEST(TracingCommandHandlerTest, SpanNameContainsTypeName) {
    ct::FakeTracer tracer;
    CommandBus bus;

    bus.register_handler<PlaceOrder>(
        tracing_command_handler<ct::FakeTracer, PlaceOrder>(tracer, [](const PlaceOrder&) {}));
    bus.send(PlaceOrder{2});

    EXPECT_FALSE(tracer.spans().empty());
}

TEST(TracingQueryHandlerTest, StartsSpanAroundQuery) {
    ct::FakeTracer tracer;
    QueryBus bus;

    bus.register_handler<GetOrderCount>(tracing_query_handler<ct::FakeTracer, GetOrderCount, int>(
        tracer, [](const GetOrderCount&) { return 42; }));

    auto result = bus.query<int>(GetOrderCount{});

    EXPECT_EQ(result, 42);
    EXPECT_EQ(tracer.spans().size(), 1u);
}

TEST(TracingQueryHandlerTest, ResultIsPassedThrough) {
    ct::FakeTracer tracer;
    QueryBus bus;

    bus.register_handler<GetOrderCount>(tracing_query_handler<ct::FakeTracer, GetOrderCount, int>(
        tracer, [](const GetOrderCount&) { return 99; }));

    EXPECT_EQ(bus.query<int>(GetOrderCount{}), 99);
}
