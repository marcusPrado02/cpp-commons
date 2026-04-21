/// @file domain_event.hpp
/// @brief Base class for all DDD domain events with UUID identity and timestamp.
#pragma once
#include "clock.hpp"
#include "identity.hpp"

#include <string>

namespace cpp_commons::kernel {

/// @brief Non-copyable base for domain events.
///
/// Derived classes call the protected constructor with a stable `event_type` string
/// (e.g. `"order.placed"`). The `event_id` and `occurred_at` are assigned automatically.
class DomainEvent {
public:
    virtual ~DomainEvent() = default;

    DomainEvent(const DomainEvent&) = delete;
    DomainEvent& operator=(const DomainEvent&) = delete;
    DomainEvent(DomainEvent&&) = default;
    DomainEvent& operator=(DomainEvent&&) = default;

    [[nodiscard]] const UUID& event_id() const noexcept { return event_id_; }
    [[nodiscard]] const std::string& event_type() const noexcept { return event_type_; }
    [[nodiscard]] const TimePoint& occurred_at() const noexcept { return occurred_at_; }
    [[nodiscard]] const std::string& correlation_id() const noexcept { return correlation_id_; }

    void set_correlation_id(std::string id) { correlation_id_ = std::move(id); }

protected:
    explicit DomainEvent(std::string event_type)
        : event_id_{UUID::generate()},
          event_type_{std::move(event_type)},
          occurred_at_{std::chrono::system_clock::now()} {}

private:
    UUID event_id_;
    std::string event_type_;
    TimePoint occurred_at_;
    std::string correlation_id_;
};

}  // namespace cpp_commons::kernel
