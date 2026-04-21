#include <outbox_publisher.hpp>

#include <cpp_commons/kernel/domain_event.hpp>
#include <cpp_commons/testing/in_memory_event_bus.hpp>

#include <gtest/gtest.h>

using namespace cpp_commons::application;

struct OrderPlaced : cpp_commons::kernel::DomainEvent {
    int order_id{};
    explicit OrderPlaced(int id) : cpp_commons::kernel::DomainEvent{"OrderPlaced"}, order_id{id} {}
};

struct PaymentReceived : cpp_commons::kernel::DomainEvent {
    double amount{};
    explicit PaymentReceived(double a)
        : cpp_commons::kernel::DomainEvent{"PaymentReceived"}, amount{a} {}
};

using Bus = cpp_commons::testing::InMemoryEventBus;

static void setup_publishers(OutboxPublisher<Bus>& outbox) {
    outbox.register_publisher<OrderPlaced>(
        [](Bus& bus, const OrderPlaced& ev) { bus.publish<OrderPlaced>(ev.order_id); });
    outbox.register_publisher<PaymentReceived>(
        [](Bus& bus, const PaymentReceived& ev) { bus.publish<PaymentReceived>(ev.amount); });
}

TEST(OutboxPublisherTest, StoreDoesNotPublishImmediately) {
    OutboxPublisher<Bus> outbox;
    Bus bus;
    setup_publishers(outbox);

    outbox.store<OrderPlaced>(42);
    EXPECT_EQ(bus.count<OrderPlaced>(), 0u);
}

TEST(OutboxPublisherTest, FlushPublishesStoredEvents) {
    OutboxPublisher<Bus> outbox;
    Bus bus;
    setup_publishers(outbox);

    outbox.store<OrderPlaced>(1);
    outbox.store<OrderPlaced>(2);
    outbox.flush(bus);

    EXPECT_EQ(bus.count<OrderPlaced>(), 2u);
}

TEST(OutboxPublisherTest, FlushClearsPendingEvents) {
    OutboxPublisher<Bus> outbox;
    Bus bus;
    setup_publishers(outbox);

    outbox.store<OrderPlaced>(1);
    outbox.flush(bus);
    outbox.flush(bus);  // second flush should not re-publish

    EXPECT_EQ(bus.count<OrderPlaced>(), 1u);
}

TEST(OutboxPublisherTest, PendingCountReflectsStored) {
    OutboxPublisher<Bus> outbox;
    Bus bus;
    EXPECT_EQ(outbox.pending_count(), 0u);
    outbox.store<OrderPlaced>(1);
    outbox.store<OrderPlaced>(2);
    EXPECT_EQ(outbox.pending_count(), 2u);
    outbox.flush(bus);
    EXPECT_EQ(outbox.pending_count(), 0u);
}

TEST(OutboxPublisherTest, MultipleEventTypesPublishedCorrectly) {
    OutboxPublisher<Bus> outbox;
    Bus bus;
    setup_publishers(outbox);

    outbox.store<OrderPlaced>(7);
    outbox.store<PaymentReceived>(99.5);
    outbox.flush(bus);

    EXPECT_EQ(bus.count<OrderPlaced>(), 1u);
    EXPECT_EQ(bus.count<PaymentReceived>(), 1u);
    EXPECT_EQ(bus.last<OrderPlaced>().order_id, 7);
    EXPECT_DOUBLE_EQ(bus.last<PaymentReceived>().amount, 99.5);
}

TEST(OutboxPublisherTest, UnregisteredEventTypeIsDropped) {
    OutboxPublisher<Bus> outbox;
    Bus bus;
    // No publisher registered for OrderPlaced

    outbox.store<OrderPlaced>(1);
    outbox.flush(bus);  // should not crash, event is silently dropped

    EXPECT_EQ(bus.count<OrderPlaced>(), 0u);
}
