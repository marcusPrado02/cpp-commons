#pragma once
#include <use_case.hpp>
#include <cpp_commons/kernel/ports/logger_port.hpp>
#include "../domain/order.hpp"
#include <string>
#include <vector>

namespace order_service::application {

struct PlaceOrderInput {
    std::string                    customer_id;
    std::vector<domain::OrderItem> items;
    std::string                    correlation_id;
};

struct PlaceOrderOutput {
    std::string  order_id;
    domain::Money total;
};

template <cpp_commons::kernel::LoggerPort Logger>
class PlaceOrderUseCase
    : public cpp_commons::application::UseCase<
          PlaceOrderInput,
          PlaceOrderOutput,
          cpp_commons::errors::DomainError> {
public:
    explicit PlaceOrderUseCase(Logger& logger) : logger_(logger) {}

    Result execute(const PlaceOrderInput& input) override {
        logger_.info("placing order for customer " + input.customer_id);

        auto uuid   = cpp_commons::kernel::UUID::generate();
        auto placed = domain::Order::place(
            cpp_commons::kernel::EntityId{uuid},
            input.customer_id,
            input.items);

        if (placed.is_err()) return Result::err(placed.error());

        auto& order  = placed.value();
        auto  events = order.pull_events();
        logger_.info("order placed, events emitted: " + std::to_string(events.size()));

        return Result::ok(PlaceOrderOutput{uuid.to_string(), order.total()});
    }

private:
    Logger& logger_;
};

} // namespace order_service::application
