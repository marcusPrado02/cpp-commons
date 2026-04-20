#include <mediator.hpp>
#include <gtest/gtest.h>

using namespace cpp_commons::application;

struct ShipOrder  { std::string order_id; };
struct GetStock   { std::string product_id; };

TEST(MediatorTest, SendDispatchesCommand) {
    Mediator m;
    std::string shipped;
    m.register_command<ShipOrder>([&](const ShipOrder& cmd) {
        shipped = cmd.order_id;
    });
    m.send(ShipOrder{"order-7"});
    EXPECT_EQ(shipped, "order-7");
}

TEST(MediatorTest, QueryDispatchesHandler) {
    Mediator m;
    m.register_query<GetStock>([](const GetStock&) { return 99; });
    EXPECT_EQ(m.query<int>(GetStock{"prod-1"}), 99);
}

TEST(MediatorTest, IndependentCommandAndQuery) {
    Mediator m;
    bool command_ran = false;
    m.register_command<ShipOrder>([&](const ShipOrder&) { command_ran = true; });
    m.register_query<GetStock>([](const GetStock&) { return 5; });

    m.send(ShipOrder{"x"});
    auto stock = m.query<int>(GetStock{"y"});

    EXPECT_TRUE(command_ran);
    EXPECT_EQ(stock, 5);
}
