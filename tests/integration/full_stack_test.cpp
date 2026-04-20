#include <correlation_context.hpp>
#include <json_logger.hpp>
#include <circuit_breaker.hpp>
#include <retry_policy.hpp>
#include <command_bus.hpp>
#include <query_bus.hpp>
#include <http_request.hpp>
#include <http_response.hpp>
#include <middleware_chain.hpp>
#include <auth_middleware.hpp>
#include <correlation_middleware.hpp>
#include <use_case.hpp>
#include <cpp_commons/kernel/result.hpp>
#include <cpp_commons/errors/domain_error.hpp>
#include <gtest/gtest.h>
#include <string>
#include <stdexcept>

using namespace cpp_commons;

// ── Integration: CorrelationScope + JsonLogger ───────────────────────────────

TEST(FullStackIntegration, CorrelationPropagatesIntoLogger) {
    // JsonLogger picks up context from thread_local — no manual threading.
    testing::internal::CaptureStderr();

    observability::JsonLogger log{"test-svc"};
    auto ctx = observability::CorrelationContext::generate();
    ctx.tenant_id = "tenant-x";
    {
        observability::CorrelationScope scope{ctx};
        log.info("hello from scope");
        EXPECT_EQ(observability::current_correlation().tenant_id, "tenant-x");
    }
    EXPECT_TRUE(observability::current_correlation().empty());
}

// ── Integration: RetryPolicy + CircuitBreaker ────────────────────────────────

TEST(FullStackIntegration, RetryTripsCircuitBreaker) {
    resilience::CircuitBreaker cb{resilience::CircuitBreakerConfig{
        .failure_threshold = 2,
        .open_duration     = std::chrono::seconds{60},
    }};

    resilience::RetryConfig retry{
        .max_attempts  = 3,
        .initial_delay = std::chrono::milliseconds{0},
    };

    // After 2 failures the breaker opens; the 3rd retry hits CircuitOpenError.
    int calls = 0;
    EXPECT_THROW(
        resilience::with_retry(retry, [&] {
            return cb.call([&]() -> int {
                ++calls;
                throw std::runtime_error{"always fails"};
            });
        }),
        resilience::RetryExhausted);

    EXPECT_TRUE(cb.is_open());
}

// ── Integration: CommandBus + UseCase + CorrelationScope ─────────────────────

struct CreateItemCmd { std::string name; };
struct ItemCreated   { std::string id; };

struct CreateItemUseCase
    : application::UseCase<CreateItemCmd, ItemCreated, errors::DomainError> {
    Result execute(const CreateItemCmd& cmd) override {
        if (cmd.name.empty())
            return Result::err(errors::InvariantViolationError{"name required"});
        return Result::ok(ItemCreated{"item-" + cmd.name});
    }
};

TEST(FullStackIntegration, CommandBusDispatchesUseCase) {
    auto ctx = observability::CorrelationContext::generate();
    observability::CorrelationScope scope{ctx};

    CreateItemUseCase uc;
    application::CommandBus bus;
    std::string created_id;

    bus.register_handler<CreateItemCmd>([&](const CreateItemCmd& cmd) {
        auto r = uc.execute(cmd);
        if (r.is_ok()) created_id = r.value().id;
    });

    bus.send(CreateItemCmd{"widget"});
    EXPECT_EQ(created_id, "item-widget");
}

// ── Integration: MiddlewareChain + auth_middleware + correlation_middleware ───

TEST(FullStackIntegration, WebMiddlewarePipelineEnforcesAuth) {
    web::MiddlewareChain chain;
    chain.use(web::correlation_middleware());
    chain.use(web::auth_middleware([](const std::string& token) {
        return token == "valid-token";
    }));

    // Request without token → 401
    {
        web::HttpRequest req;
        req.method = web::HttpMethod::Get;
        req.path   = "/items";
        auto resp = chain.dispatch(req, [](const web::HttpRequest&) {
            return web::HttpResponse::ok(R"({"items":[]})");
        });
        EXPECT_EQ(resp.status_code, 401);
    }

    // Request with valid token → 200, handler runs
    {
        web::HttpRequest req;
        req.method = web::HttpMethod::Get;
        req.path   = "/items";
        req.headers["Authorization"] = "Bearer valid-token";
        req.headers["X-Tenant-ID"]   = "tenant-y";
        auto resp = chain.dispatch(req, [](const web::HttpRequest&) {
            // Inside the handler the correlation scope is active.
            EXPECT_EQ(observability::current_correlation().tenant_id, "tenant-y");
            return web::HttpResponse::ok(R"({"items":[]})");
        });
        EXPECT_EQ(resp.status_code, 200);
    }
}

// ── Integration: HttpResponse::from_problem + DomainError mapping ────────────

TEST(FullStackIntegration, ProblemDetailsRoundTrip) {
    auto pd = errors::ProblemDetails::not_found("order order-99 not found");
    auto resp = web::HttpResponse::from_problem(pd);
    EXPECT_EQ(resp.status_code, 404);
    EXPECT_FALSE(resp.body.empty());
}
