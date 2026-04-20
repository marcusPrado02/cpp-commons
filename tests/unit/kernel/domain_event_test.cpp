#include <cpp_commons/kernel/domain_event.hpp>
#include <gtest/gtest.h>

using cpp_commons::kernel::DomainEvent;

class OrderCreated : public DomainEvent {
public:
    explicit OrderCreated(std::string order_id)
        : DomainEvent{"OrderCreated"}, order_id_{std::move(order_id)} {}
    const std::string& order_id() const { return order_id_; }
private:
    std::string order_id_;
};

TEST(DomainEventTest, HasEventId) {
    OrderCreated e{"ord-1"};
    EXPECT_FALSE(e.event_id().to_string().empty());
}

TEST(DomainEventTest, HasEventType) {
    OrderCreated e{"ord-1"};
    EXPECT_EQ(e.event_type(), "OrderCreated");
}

TEST(DomainEventTest, OccurredAtIsSet) {
    auto before = std::chrono::system_clock::now();
    OrderCreated e{"ord-1"};
    auto after  = std::chrono::system_clock::now();
    EXPECT_GE(e.occurred_at(), before);
    EXPECT_LE(e.occurred_at(), after);
}

TEST(DomainEventTest, CorrelationIdSettable) {
    OrderCreated e{"ord-1"};
    e.set_correlation_id("corr-123");
    EXPECT_EQ(e.correlation_id(), "corr-123");
}

TEST(DomainEventTest, UniqueEventIds) {
    OrderCreated a{"ord-1"}, b{"ord-2"};
    EXPECT_NE(a.event_id(), b.event_id());
}
