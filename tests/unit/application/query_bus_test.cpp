#include <query_bus.hpp>
#include <string>

#include <gtest/gtest.h>

using namespace cpp_commons::application;

struct GetUser {
    int id;
};
struct UserDto {
    std::string name;
};

struct GetCount {};

TEST(QueryBusTest, DispatchesRegisteredHandler) {
    QueryBus bus;
    bus.register_handler<GetUser>(
        [](const GetUser& q) -> UserDto { return UserDto{"user-" + std::to_string(q.id)}; });

    auto result = bus.query<UserDto>(GetUser{42});
    EXPECT_EQ(result.name, "user-42");
}

TEST(QueryBusTest, ThrowsWhenNoHandlerRegistered) {
    QueryBus bus;
    EXPECT_THROW(bus.query<UserDto>(GetUser{1}), QueryNotRegistered);
}

TEST(QueryBusTest, MultipleHandlersCoexist) {
    QueryBus bus;
    bus.register_handler<GetUser>(
        [](const GetUser& q) -> UserDto { return UserDto{"u" + std::to_string(q.id)}; });
    bus.register_handler<GetCount>([](const GetCount&) -> int { return 7; });

    EXPECT_EQ(bus.query<UserDto>(GetUser{1}).name, "u1");
    EXPECT_EQ(bus.query<int>(GetCount{}), 7);
}

TEST(QueryBusTest, LastRegistrationWins) {
    QueryBus bus;
    bus.register_handler<GetUser>([](const GetUser&) -> UserDto { return {"first"}; });
    bus.register_handler<GetUser>([](const GetUser&) -> UserDto { return {"second"}; });

    EXPECT_EQ(bus.query<UserDto>(GetUser{0}).name, "second");
}

TEST(QueryBusTest, HandlerCalledOnEachQuery) {
    QueryBus bus;
    int call_count = 0;
    bus.register_handler<GetCount>([&](const GetCount&) -> int { return ++call_count; });

    EXPECT_EQ(bus.query<int>(GetCount{}), 1);
    EXPECT_EQ(bus.query<int>(GetCount{}), 2);
}
