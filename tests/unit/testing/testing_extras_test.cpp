#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include <cpp_commons/kernel/domain_event.hpp>
#include <cpp_commons/testing/fake_repository.hpp>
#include <cpp_commons/testing/fake_tracer.hpp>
#include <cpp_commons/testing/in_memory_event_bus.hpp>

#include <gtest/gtest.h>

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
    SUCCEED();  // concept satisfied at compile time via static_assert
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
    bus.subscribe<ItemCreatedEvent>([&](const ItemCreatedEvent& e) { received = e.item_id; });
    bus.publish<ItemCreatedEvent>("item-42");
    EXPECT_EQ(received, "item-42");
}

TEST(InMemoryEventBusTest, ClearResetsAll) {
    ct::InMemoryEventBus bus;
    bus.publish<ItemCreatedEvent>("x");
    bus.clear();
    EXPECT_EQ(bus.count<ItemCreatedEvent>(), 0u);
}

// ── Concurrency ───────────────────────────────────────────────────────────────

TEST(InMemoryEventBusConcurrencyTest, ConcurrentPublishNoDataRace) {
    constexpr int kThreads = 4;
    constexpr int kPerThread = 25;

    ct::InMemoryEventBus bus;
    std::atomic<int> handler_calls{0};

    bus.subscribe<ItemCreatedEvent>(
        [&](const ItemCreatedEvent&) { handler_calls.fetch_add(1, std::memory_order_relaxed); });

    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&, i] {
            for (int j = 0; j < kPerThread; ++j)
                bus.publish<ItemCreatedEvent>("item-" + std::to_string(i * kPerThread + j));
        });
    }
    for (auto& t : threads)
        t.join();

    EXPECT_EQ(bus.count<ItemCreatedEvent>(), static_cast<std::size_t>(kThreads * kPerThread));
    EXPECT_EQ(handler_calls.load(), kThreads * kPerThread);
}

TEST(InMemoryEventBusConcurrencyTest, ConcurrentSubscribeAndPublish) {
    ct::InMemoryEventBus bus;
    std::atomic<int> received{0};

    // Subscribers register concurrently while publish happens
    std::thread subscriber([&] {
        for (int i = 0; i < 10; ++i)
            bus.subscribe<ItemCreatedEvent>(
                [&](const ItemCreatedEvent&) { received.fetch_add(1, std::memory_order_relaxed); });
    });

    std::thread publisher([&] {
        for (int i = 0; i < 10; ++i)
            bus.publish<ItemCreatedEvent>("x");
    });

    subscriber.join();
    publisher.join();

    // At least 10 events published; exact handler count depends on race outcome — just no crash
    EXPECT_GE(bus.count<ItemCreatedEvent>(), 10u);
}
