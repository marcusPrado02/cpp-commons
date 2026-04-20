#include <command_bus.hpp>
#include <query_bus.hpp>
#include <gtest/gtest.h>

using namespace cpp_commons::application;

struct CreateOrder { int product_id; int qty; };
struct OrderCount {};

TEST(CommandBusTest, DispatchesToHandler) {
    CommandBus bus;
    int received_id = 0;
    bus.register_handler<CreateOrder>([&](const CreateOrder& cmd) {
        received_id = cmd.product_id;
    });
    bus.send(CreateOrder{42, 3});
    EXPECT_EQ(received_id, 42);
}

TEST(CommandBusTest, ThrowsWhenHandlerMissing) {
    CommandBus bus;
    EXPECT_THROW(bus.send(CreateOrder{1, 1}), CommandNotRegistered);
}

TEST(QueryBusTest, DispatchesToHandlerAndReturnsResult) {
    QueryBus bus;
    bus.register_handler<OrderCount>([](const OrderCount&) { return 99; });
    auto count = bus.query<int>(OrderCount{});
    EXPECT_EQ(count, 99);
}

TEST(QueryBusTest, ThrowsWhenHandlerMissing) {
    QueryBus bus;
    EXPECT_THROW([[maybe_unused]] auto _ = bus.query<int>(OrderCount{}), QueryNotRegistered);
}
