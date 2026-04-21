# cpp-commons

Production-ready C++20 building blocks for microservice backends: domain primitives, hexagonal architecture ports, resilience patterns, structured logging, and HTTP middleware.

## Quick Start

```bash
cmake --preset dev
cmake --build build/dev
ctest --preset dev
```

Requires: GCC 13+ or Clang 17+, CMake 3.28+, Ninja.

## Modules

### Kernel (header-only)

Core domain primitives with zero runtime overhead.

| Header | What it provides |
|--------|-----------------|
| `kernel/result.hpp` | `Result<T,E>` — railway-oriented error handling wrapping `tl::expected`. Methods: `map`, `map_err`, `and_then`, `flatten`. |
| `kernel/option.hpp` | `Option<T>` — functional optional. Methods: `map`, `filter`, `value_or_else`. |
| `kernel/identity.hpp` | `UUID` — type-safe RFC 4122 v4 UUID. `StrongId<Tag>` — tag-dispatched identifier. `std::hash` specializations included. |
| `kernel/value_object.hpp` | `ValueObject<Derived>` — CRTP base for equality/hash via `fields()` tuple. |
| `kernel/money.hpp` | `Money` — ISO 4217 currency + int64 cents with overflow-safe arithmetic. |
| `kernel/email.hpp` | `Email::parse(sv)` → `Result<Email, ValidationError>`. |
| `kernel/phone_number.hpp` | `PhoneNumber::parse(sv)` → `Result<PhoneNumber, ValidationError>` — E.164 format. |
| `kernel/specification.hpp` | `Specification<T>` — composable predicates (`&&`, `\|\|`, `!`). |
| `kernel/aggregate_root.hpp` | `AggregateRoot<Id>` — DDD aggregate with event accumulation. |
| `kernel/ports/` | Concepts: `LoggerPort`, `MetricsPort`, `TracerPort`, `RepositoryPort`. |

### Errors

```cpp
#include <cpp_commons/errors/domain_error.hpp>

Result<Item, DomainError>::err(NotFoundError{"item-1"});
// → ProblemDetails::not_found("item-1") → HTTP 404 application/problem+json
```

| Type | HTTP Status |
|------|------------|
| `NotFoundError` | 404 |
| `InvariantViolationError` | 422 |
| `ConflictError` | 409 |
| `UnauthorizedError` | 401 |
| `ValidationError` | 422 |

### Observability

```cpp
#include <json_logger.hpp>
#include <prometheus_metrics.hpp>
#include <otel_tracer.hpp>
#include <correlation_context.hpp>

// Structured JSON logging with correlation context propagation
JsonLogger log{"my-service"};
{
    CorrelationScope scope{CorrelationContext::generate()};
    log.info("order placed", {{"order_id", "ord-42"}, {"amount", "4200"}});
}

// Prometheus text-format metrics
PrometheusMetrics metrics;
metrics.increment("http_requests_total");
metrics.gauge("active_connections", 12.0);
metrics.histogram("request_duration_ms", 45.0);
std::string scrape_body = metrics.render();  // for /metrics endpoint

// W3C Trace Context
CorrelationContext ctx;
ctx.set_traceparent("00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01");
auto tp = ctx.traceparent();  // "00-{trace_id}-{span_id}-01"

// OTel-compatible span tracing
OtelTracer tracer{[](SpanRecord r) { /* export to collector */ }};
tracer.start_span("db.query");
// ... work ...
tracer.end_span();
```

### Resilience

```cpp
#include <retry_policy.hpp>
#include <circuit_breaker.hpp>
#include <rate_limiter.hpp>
#include <bulkhead.hpp>

// Retry with exponential backoff + full jitter + exception filter
RetryConfig cfg{
    .max_attempts  = 3,
    .initial_delay = 100ms,
    .jitter        = JitterStrategy::Full,
    .should_retry  = retry_on<TransientError>(),
};
auto result = with_retry(cfg, [&] { return call_remote(); });

// Circuit breaker
CircuitBreaker cb{CircuitBreakerConfig{.failure_threshold=5, .open_duration=30s}};
cb.call([&] { return db.query(sql); });

// Token bucket rate limiter
RateLimiter limiter{100.0, 10.0};  // 100 max, refill 10/sec
if (limiter.try_acquire()) { /* allowed */ }
```

### Application (CQRS)

```cpp
#include <command_bus.hpp>
#include <query_bus.hpp>
#include <use_case.hpp>

CommandBus bus;
bus.register_handler<CreateOrderCmd>([&](const CreateOrderCmd& cmd) { /* ... */ });
bus.send(CreateOrderCmd{"item-1", 3});

QueryBus qbus;
qbus.register_handler<GetOrderQuery>([](const GetOrderQuery& q) -> OrderDto { /* ... */ });
auto dto = qbus.query<OrderDto>(GetOrderQuery{order_id});
```

### Web (HTTP)

```cpp
#include <middleware_chain.hpp>
#include <cors_middleware.hpp>
#include <auth_middleware.hpp>
#include <http_response.hpp>

MiddlewareChain chain;
chain.use(cors_middleware(CorsOptions{.allowed_origins={"https://app.example.com"}}));
chain.use(correlation_middleware());
chain.use(auth_middleware([](const std::string& token) { return jwt.verify(token); }));

auto resp = chain.dispatch(req, [](const HttpRequest& r) {
    return HttpResponse::ok(R"({"status":"ok"})");
});
```

### Security

```cpp
#include <jwt_signer.hpp>
#include <pii_redactor.hpp>

// HS256 JWT sign + verify
JwtSigner signer{"my-secret-key"};
auto token = signer.sign({{"sub", "user-1"}, {"role", "admin"}});
auto claims = signer.verify(token);  // throws JwtError on invalid sig

// PII redaction for logs
auto safe = PiiRedactor::redact_all("email user@example.com card 4111 1111 1111 1111");
// → "email [EMAIL] card [CARD]"
```

### Config

```cpp
#include <settings.hpp>

Settings s;
auto db_url  = s.require("DATABASE_URL");
auto timeout = s.env_as<std::chrono::milliseconds>("TIMEOUT", "500ms");

// Accumulate all errors before throwing
ConfigValidator v;
auto host = v.require("DB_HOST");
auto port = v.require_int("DB_PORT");
auto ttl  = v.require_duration("CACHE_TTL");
v.check();  // throws ConfigError with all missing vars listed
```

## Testing Fakes

All ports have in-memory test doubles in `include/cpp_commons/testing/`:

```cpp
#include <cpp_commons/testing/fake_logger.hpp>
#include <cpp_commons/testing/fake_metrics.hpp>
#include <cpp_commons/testing/fake_tracer.hpp>
#include <cpp_commons/testing/fake_repository.hpp>

FakeLogger logger;
logger.info("hello");
EXPECT_EQ(logger.last_message(), "hello");
```

## Build Presets

| Preset | Use |
|--------|-----|
| `dev` | Local development, debug, compile_commands.json |
| `ci` | CI build — UBSan + `-Werror` |
| `ci-asan` | CI build — AddressSanitizer |
| `coverage` | gcov coverage (used by `make coverage`) |

## CI

GitHub Actions runs on push/PR:
- **format** — clang-format-17 check
- **build** — GCC-13 + Clang-17 × ci + ci-asan presets
- **tidy** — clang-tidy-17
- **coverage** — lcov with 80% line threshold

## License

MIT
