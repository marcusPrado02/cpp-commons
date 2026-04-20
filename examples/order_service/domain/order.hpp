#pragma once
#include <cpp_commons/kernel/aggregate_root.hpp>
#include <cpp_commons/kernel/domain_event.hpp>
#include <cpp_commons/kernel/result.hpp>
#include <cpp_commons/errors/domain_error.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace order_service::domain {

// ── Value objects ────────────────────────────────────────────────────────────

struct Money {
    int64_t cents{0};
    std::string currency{"BRL"};

    [[nodiscard]] static Money of(int64_t cents, std::string currency) {
        return {cents, std::move(currency)};
    }
    [[nodiscard]] bool operator==(const Money& o) const noexcept {
        return cents == o.cents && currency == o.currency;
    }
};

struct OrderItem {
    std::string product_id;
    uint32_t    quantity{0};
    Money       unit_price;
};

// ── Domain events ────────────────────────────────────────────────────────────

struct OrderPlaced : cpp_commons::kernel::DomainEvent {
    OrderPlaced() : DomainEvent("OrderPlaced") {}
};

struct OrderCancelled : cpp_commons::kernel::DomainEvent {
    OrderCancelled() : DomainEvent("OrderCancelled") {}
};

// ── Aggregate ────────────────────────────────────────────────────────────────

enum class OrderStatus { Pending, Confirmed, Cancelled };

using OrderResult = cpp_commons::kernel::Result<class Order, cpp_commons::errors::DomainError>;

class Order : public cpp_commons::kernel::AggregateRoot<cpp_commons::kernel::EntityId> {
public:
    using Id     = cpp_commons::kernel::EntityId;
    using Err    = cpp_commons::errors::DomainError;

    template <typename T>
    using Result = cpp_commons::kernel::Result<T, Err>;

    [[nodiscard]] static Result<Order>
    place(Id id, std::string customer_id, std::vector<OrderItem> items) {
        if (items.empty())
            return Result<Order>::err(
                cpp_commons::errors::InvariantViolationError{"order must have at least one item"});
        Order o{std::move(id), std::move(customer_id), std::move(items)};
        o.record<OrderPlaced>();
        return Result<Order>::ok(std::move(o));
    }

    [[nodiscard]] Result<bool> cancel() {
        if (status_ == OrderStatus::Cancelled)
            return Result<bool>::err(
                cpp_commons::errors::InvariantViolationError{"order is already cancelled"});
        status_ = OrderStatus::Cancelled;
        record<OrderCancelled>();
        return Result<bool>::ok(true);
    }

    [[nodiscard]] OrderStatus status() const noexcept { return status_; }
    [[nodiscard]] const std::string& customer_id() const noexcept { return customer_id_; }
    [[nodiscard]] const std::vector<OrderItem>& items() const noexcept { return items_; }

    [[nodiscard]] Money total() const noexcept {
        int64_t sum = 0;
        for (const auto& item : items_)
            sum += item.unit_price.cents * static_cast<int64_t>(item.quantity);
        return Money::of(sum, items_.empty() ? "BRL" : items_[0].unit_price.currency);
    }

private:
    Order(Id id, std::string customer_id, std::vector<OrderItem> items)
        : AggregateRoot(std::move(id))
        , customer_id_(std::move(customer_id))
        , items_(std::move(items)) {}

    std::string            customer_id_;
    std::vector<OrderItem> items_;
    OrderStatus            status_{OrderStatus::Pending};
};

} // namespace order_service::domain
