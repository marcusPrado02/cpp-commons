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

TEST(QueryBusTest, MultipleQueryTypesRegistered) {
    struct ProductCount {};
    struct UserCount {};
    QueryBus bus;
    bus.register_handler<ProductCount>([](const ProductCount&) { return 5; });
    bus.register_handler<UserCount>([](const UserCount&) { return 12; });
    EXPECT_EQ(bus.query<int>(ProductCount{}), 5);
    EXPECT_EQ(bus.query<int>(UserCount{}), 12);
}

TEST(QueryBusTest, HandlerReceivesQueryFields) {
    struct FindById { int id; };
    QueryBus bus;
    int captured = 0;
    bus.register_handler<FindById>([&](const FindById& q) {
        captured = q.id;
        return std::string{"found"};
    });
    auto result = bus.query<std::string>(FindById{77});
    EXPECT_EQ(result, "found");
    EXPECT_EQ(captured, 77);
}

TEST(QueryBusTest, LastRegisteredHandlerWins) {
    QueryBus bus;
    bus.register_handler<OrderCount>([](const OrderCount&) { return 1; });
    bus.register_handler<OrderCount>([](const OrderCount&) { return 2; });
    EXPECT_EQ(bus.query<int>(OrderCount{}), 2);
}

TEST(QueryBusTest, StringResultType) {
    struct Greeting {};
    QueryBus bus;
    bus.register_handler<Greeting>([](const Greeting&) {
        return std::string{"hello"};
    });
    EXPECT_EQ(bus.query<std::string>(Greeting{}), "hello");
}
