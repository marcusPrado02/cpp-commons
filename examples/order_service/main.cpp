#include <json_logger.hpp>
#include <correlation_context.hpp>
#include <circuit_breaker.hpp>
#include <command_bus.hpp>
#include "domain/order.hpp"
#include "application/place_order.hpp"
#include <format>
#include <iostream>

namespace obs  = cpp_commons::observability;
namespace res  = cpp_commons::resilience;
namespace app  = cpp_commons::application;
namespace dom  = order_service::domain;
namespace svc  = order_service::application;

// ── Command ──────────────────────────────────────────────────────────────────

struct PlaceOrderCommand {
    std::string customer_id;
    std::vector<dom::OrderItem> items;
};

// ── Entry point ──────────────────────────────────────────────────────────────

int main() {
    // 1. Structured JSON logger (spdlog-backed, concept-dispatched)
    obs::JsonLogger logger{"order-service"};

    // 2. Correlation scope — installs IDs into thread-local storage so
    //    JsonLogger picks them up automatically on every log call.
    auto ctx = obs::CorrelationContext::generate();
    ctx.tenant_id = "tenant-acme";
    obs::CorrelationScope scope{ctx};

    logger.info(std::format("startup correlation_id={}", ctx.correlation_id));

    // 3. Use-case (template, zero vtable overhead for logger)
    svc::PlaceOrderUseCase<obs::JsonLogger> use_case{logger};

    // 4. Circuit breaker — wraps the use-case call to simulate
    //    protecting a downstream dependency.
    res::CircuitBreaker cb{res::CircuitBreakerConfig{
        .failure_threshold = 3,
        .open_duration     = std::chrono::seconds{5},
    }};

    // 5. Command bus — decouples caller from handler
    app::CommandBus bus;
    bus.register_handler<PlaceOrderCommand>([&](const PlaceOrderCommand& cmd) {
        svc::PlaceOrderInput input{
            cmd.customer_id,
            cmd.items,
            obs::current_correlation().correlation_id,
        };

        auto result = cb.call([&] { return use_case.execute(input); });

        if (result.is_ok()) {
            logger.info(std::format(
                "order {} placed — total {} cents",
                result.value().order_id,
                result.value().total.cents));
        } else {
            logger.error(std::string{"failed to place order: "} + result.error().what());
        }
    });

    // ── Scenario 1: happy path ───────────────────────────────────────────────
    logger.info("--- scenario: happy path ---");
    bus.send(PlaceOrderCommand{
        "customer-42",
        {
            dom::OrderItem{"prod-001", 2, dom::Money::of(1500, "BRL")},
            dom::OrderItem{"prod-002", 1, dom::Money::of(3000, "BRL")},
        },
    });

    // ── Scenario 2: empty items → domain invariant error ─────────────────────
    logger.info("--- scenario: empty items ---");
    bus.send(PlaceOrderCommand{"customer-42", {}});

    // ── Scenario 3: nested correlation scope ─────────────────────────────────
    logger.info("--- scenario: nested correlation scope ---");
    {
        auto inner_ctx = obs::CorrelationContext::generate();
        inner_ctx.tenant_id = "tenant-beta";
        obs::CorrelationScope inner_scope{inner_ctx};
        logger.info(std::format(
            "inner scope cid={} tenant={}",
            obs::current_correlation().correlation_id,
            obs::current_correlation().tenant_id));
    }
    logger.info(std::format(
        "restored cid={} tenant={}",
        obs::current_correlation().correlation_id,
        obs::current_correlation().tenant_id));

    return 0;
}
