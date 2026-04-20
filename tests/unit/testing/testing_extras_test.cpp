#include <cpp_commons/testing/fake_tracer.hpp>
#include <cpp_commons/testing/fake_repository.hpp>
#include <cpp_commons/testing/in_memory_event_bus.hpp>
#include <cpp_commons/kernel/domain_event.hpp>
#include <gtest/gtest.h>
#include <string>

namespace ct = cpp_commons::testing;

// ── FakeTracer ────────────────────────────────────────────────────────────────

TEST(FakeTracerTest, RecordsSpans) {
    ct::FakeTracer tracer;
    tracer.start_span("db.query");
    tracer.end_span();
    tracer.start_span("http.call");
    tracer.end_span();

    EXPECT_EQ(tracer.spans().size(), 2u);
    EXPECT_TRUE(tracer.has_span("db.query"));
    EXPECT_FALSE(tracer.has_span("grpc.call"));
}

TEST(FakeTracerTest, ClearResetsState) {
    ct::FakeTracer tracer;
    tracer.start_span("x");
    tracer.clear();
    EXPECT_TRUE(tracer.spans().empty());
}

TEST(NoopTracerTest, CompilesAndSatisfiesConcept) {
    ct::NoopTracer t;
    t.start_span("anything");
    t.end_span();
    SUCCEED(); // concept satisfied at compile time via static_assert
}

// ── InMemoryEventBus ──────────────────────────────────────────────────────────

struct ItemCreatedEvent : cpp_commons::kernel::DomainEvent {
    std::string item_id;
    explicit ItemCreatedEvent(std::string id)
        : DomainEvent("ItemCreated"), item_id(std::move(id)) {}
};

TEST(InMemoryEventBusTest, PublishAndCount) {
    ct::InMemoryEventBus bus;
    bus.publish<ItemCreatedEvent>("item-1");
    bus.publish<ItemCreatedEvent>("item-2");
    EXPECT_EQ(bus.count<ItemCreatedEvent>(), 2u);
}

TEST(InMemoryEventBusTest, LastReturnsLatestEvent) {
    ct::InMemoryEventBus bus;
    bus.publish<ItemCreatedEvent>("item-1");
    bus.publish<ItemCreatedEvent>("item-99");
    EXPECT_EQ(bus.last<ItemCreatedEvent>().item_id, "item-99");
}

TEST(InMemoryEventBusTest, SubscriberReceivesEvent) {
    ct::InMemoryEventBus bus;
    std::string received;
    bus.subscribe<ItemCreatedEvent>([&](const ItemCreatedEvent& e) {
        received = e.item_id;
    });
    bus.publish<ItemCreatedEvent>("item-42");
    EXPECT_EQ(received, "item-42");
}

TEST(InMemoryEventBusTest, ClearResetsAll) {
    ct::InMemoryEventBus bus;
    bus.publish<ItemCreatedEvent>("x");
    bus.clear();
    EXPECT_EQ(bus.count<ItemCreatedEvent>(), 0u);
}
