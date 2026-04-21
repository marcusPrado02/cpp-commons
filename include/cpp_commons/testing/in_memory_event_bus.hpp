/// @file in_memory_event_bus.hpp
/// @brief InMemoryEventBus — thread-safe event store and dispatcher for unit tests.
#pragma once
#include <cpp_commons/kernel/domain_event.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace cpp_commons::testing {

/// @brief Publishes domain events to in-process subscribers and stores them for assertion.
///
/// Use `count<E>()` / `last<E>()` in tests; `subscribe<E>(handler)` for side-effects.
class InMemoryEventBus {
public:
    template <typename E>
    void subscribe(std::function<void(const E&)> handler) {
        std::lock_guard lock{mutex_};
        handlers_[std::type_index(typeid(E))].push_back(
            [h = std::move(handler)](const kernel::DomainEvent& ev) {
                h(static_cast<const E&>(ev));
            });
    }

    template <typename E, typename... Args>
    void publish(Args&&... args) {
        auto key = std::type_index(typeid(E));
        auto event_ptr = std::make_unique<E>(std::forward<Args>(args)...);
        const E& ref = *event_ptr;

        std::vector<HandlerFn> local_handlers;
        {
            std::lock_guard lock{mutex_};
            published_[key].push_back(std::move(event_ptr));
            if (auto it = handlers_.find(key); it != handlers_.end())
                local_handlers = it->second;
        }
        for (const auto& h : local_handlers)
            h(ref);
    }

    template <typename E>
    [[nodiscard]] std::size_t count() const {
        std::lock_guard lock{mutex_};
        auto it = published_.find(std::type_index(typeid(E)));
        return it == published_.end() ? 0 : it->second.size();
    }

    template <typename E>
    [[nodiscard]] const E& last() const {
        std::lock_guard lock{mutex_};
        auto it = published_.find(std::type_index(typeid(E)));
        if (it == published_.end() || it->second.empty())
            throw std::out_of_range{"no events of this type published"};
        return static_cast<const E&>(*it->second.back());
    }

    void clear() {
        std::lock_guard lock{mutex_};
        published_.clear();
        handlers_.clear();
    }

private:
    using HandlerFn = std::function<void(const kernel::DomainEvent&)>;
    mutable std::mutex mutex_;
    std::unordered_map<std::type_index, std::vector<std::unique_ptr<kernel::DomainEvent>>> published_;
    std::unordered_map<std::type_index, std::vector<HandlerFn>> handlers_;
};

} // namespace cpp_commons::testing
