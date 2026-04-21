#pragma once
#include <functional>
#include <memory>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include <cpp_commons/kernel/domain_event.hpp>

namespace cpp_commons::application {

// Outbox pattern: persist events before publishing, then flush to the bus.
// EventBus must have: publish<E>(Args&&...) or accept a typed callable.
//
// Usage:
//   OutboxPublisher outbox;
//   outbox.store<OrderPlaced>(...);   // safe to call inside a "transaction"
//   outbox.flush(bus);               // call after commit; at-least-once delivery
//   outbox.flush(bus);               // idempotent if no new events
template <typename EventBus>
class OutboxPublisher {
public:
    using PublishFn = std::function<void(EventBus&, const kernel::DomainEvent&)>;

    // Register a publisher for a concrete event type.
    // Called automatically by store<E>() if PublishFn is registered first.
    template <typename E>
    void register_publisher(std::function<void(EventBus&, const E&)> fn) {
        publishers_[std::type_index(typeid(E))] =
            [f = std::move(fn)](EventBus& bus, const kernel::DomainEvent& ev) {
                f(bus, static_cast<const E&>(ev));
            };
    }

    // Store an event in the outbox. Thread-safe.
    template <typename E, typename... Args>
    void store(Args&&... args) {
        std::lock_guard lock{mutex_};
        outbox_.push_back(
            {std::type_index(typeid(E)), std::make_unique<E>(std::forward<Args>(args)...)});
    }

    // Publish all pending events to the bus, then clear. Thread-safe.
    // Events are removed only after the publisher fn returns (at-least-once).
    void flush(EventBus& bus) {
        std::vector<Entry> pending;
        {
            std::lock_guard lock{mutex_};
            pending.swap(outbox_);
        }
        for (const auto& entry : pending) {
            auto it = publishers_.find(entry.type);
            if (it != publishers_.end())
                it->second(bus, *entry.event);
        }
    }

    [[nodiscard]] std::size_t pending_count() const {
        std::lock_guard lock{mutex_};
        return outbox_.size();
    }

private:
    struct Entry {
        std::type_index type;
        std::unique_ptr<kernel::DomainEvent> event;
    };

    mutable std::mutex mutex_;
    std::vector<Entry> outbox_;
    std::unordered_map<std::type_index, PublishFn> publishers_;
};

}  // namespace cpp_commons::application
