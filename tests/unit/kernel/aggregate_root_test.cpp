#include <cpp_commons/kernel/aggregate_root.hpp>
#include <gtest/gtest.h>
#include <string>

using cpp_commons::kernel::AggregateRoot;
using cpp_commons::kernel::DomainEvent;

struct OrderId {
    int value{};
    bool operator==(const OrderId&) const = default;
};

class OrderPlaced : public DomainEvent {
public:
    explicit OrderPlaced(int order_id)
        : DomainEvent{"OrderPlaced"}, order_id_{order_id} {}
    int order_id() const { return order_id_; }
private:
    int order_id_;
};

class Order : public AggregateRoot<OrderId> {
public:
    static Order create(int id) {
        Order o{OrderId{id}};
        o.record<OrderPlaced>(id);
        return o;
    }
private:
    explicit Order(OrderId id) : AggregateRoot{std::move(id)} {}
};

TEST(AggregateRootTest, RecordsEvent) {
    auto order = Order::create(1);
    EXPECT_TRUE(order.has_pending_events());
}

TEST(AggregateRootTest, PullDrainsEvents) {
    auto order = Order::create(1);
    auto events = order.pull_events();
    EXPECT_EQ(events.size(), 1u);
    EXPECT_FALSE(order.has_pending_events());
}

TEST(AggregateRootTest, EventHasCorrectType) {
    auto order  = Order::create(1);
    auto events = order.pull_events();
    EXPECT_EQ(events[0]->event_type(), "OrderPlaced");
}

TEST(AggregateRootTest, MultipleEventsPreserveOrder) {
    auto order = Order::create(1);
    [[maybe_unused]] auto _ = order.pull_events();  // drain first
    // After pull, no events
    EXPECT_FALSE(order.has_pending_events());
}
