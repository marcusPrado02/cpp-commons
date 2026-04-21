# cpp-commons

[![CI](https://github.com/example/cpp-commons/actions/workflows/ci.yml/badge.svg)](https://github.com/example/cpp-commons/actions/workflows/ci.yml)
[![Coverage](https://img.shields.io/badge/coverage-80%25-brightgreen)](https://github.com/example/cpp-commons/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)

Production-ready C++20 building blocks for microservice backends: domain primitives, hexagonal architecture ports, resilience patterns, structured logging, and HTTP middleware.

---

## Add via FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(cpp_commons
    GIT_REPOSITORY https://github.com/example/cpp-commons.git
    GIT_TAG        v0.1.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(cpp_commons)

target_link_libraries(my_service PRIVATE
    cpp_commons::kernel
    cpp_commons::errors
    cpp_commons::observability
    cpp_commons::resilience
    cpp_commons::application
    cpp_commons::web
    cpp_commons::security
)
```

---

## End-to-End Example

```cpp
// PlaceOrderUseCase — all cpp-commons abstractions wired together
#include <use_case.hpp>
#include <json_logger.hpp>
#include <correlation_context.hpp>
#include <retry_policy.hpp>
using namespace cpp_commons;
using namespace std::chrono_literals;

struct PlaceOrderCmd { std::string item_id; int quantity; };
struct OrderId       { std::string value; };

class PlaceOrderUseCase
    : public application::UseCase<PlaceOrderCmd, OrderId, errors::DomainError> {
public:
    explicit PlaceOrderUseCase(observability::JsonLogger& log) : log_{log} {}

    Result execute(const PlaceOrderCmd& cmd) override {
        if (cmd.quantity <= 0)
            return Result::err(errors::ValidationError{"quantity must be positive"});

        log_.info("placing order", {{"item", cmd.item_id}});

        resilience::RetryConfig cfg{.max_attempts = 3, .initial_delay = 50ms,
                                    .jitter = resilience::JitterStrategy::Full};
        auto id = resilience::with_retry(cfg, [&] { return repo_.save(cmd); });
        return Result::ok(OrderId{id});
    }
private:
    observability::JsonLogger& log_;
    struct { std::string save(const PlaceOrderCmd&) { return "ord-1"; } } repo_;
};
```

---

## Quick Start

```bash
cmake --preset dev
cmake --build build/dev
ctest --preset dev
```

Requires: GCC 13+ or Clang 17+, CMake 3.28+, Ninja.

---

## Modules

### Kernel (header-only)

| Header | What it provides |
|--------|-----------------|
| `kernel/result.hpp` | `Result<T,E>` — railway-oriented error handling. `map`, `map_err`, `and_then`, `flatten`. |
| `kernel/option.hpp` | `Option<T>` — functional optional. `map`, `filter`, `value_or_else`. |
| `kernel/identity.hpp` | `UUID` — RFC 4122 v4. `StrongId<Tag>` — tag-dispatched id. `std::hash` included. |
| `kernel/value_object.hpp` | `ValueObject<Derived>` — CRTP equality/hash via `fields()` tuple. |
| `kernel/money.hpp` | `Money` — ISO 4217 currency + int64 cents, overflow-safe arithmetic. |
| `kernel/email.hpp` | `Email::parse(sv)` → `Result<Email, ValidationError>`. |
| `kernel/phone_number.hpp` | `PhoneNumber::parse(sv)` — E.164 validation. |
| `kernel/specification.hpp` | `Specification<T>` — composable predicates (`&&`, `\|\|`, `!`). `to_string()` for rejection logs. |
| `kernel/aggregate_root.hpp` | `AggregateRoot<Id>` — DDD aggregate with domain event accumulation. |
| `kernel/deadline.hpp` | `Deadline` — absolute time point for cross-boundary propagation. |
| `kernel/ports/` | Concepts: `LoggerPort`, `MetricsPort`, `TracerPort`, `RepositoryPort`. |

### Errors

```cpp
#include <cpp_commons/errors/domain_error.hpp>

auto result = Result<Item, DomainError>::err(NotFoundError{"item-1"});
auto resp   = HttpResponse::from_error(result.error());  // → 404 + problem+json
```

| Type | HTTP |
|------|------|
| `NotFoundError` | 404 |
| `ValidationError` / `InvariantViolationError` | 422 |
| `UnauthorizedError` | 401 |
| `ForbiddenError` | 403 |
| `ConflictError` | 409 |
| `RateLimitError` | 429 |
| `TimeoutError` | 504 |

### Observability

```cpp
#include <json_logger.hpp>
#include <prometheus_metrics.hpp>
#include <correlation_context.hpp>

// stdout JSON
JsonLogger log{"svc"};
log.info("order placed", {{"order_id", "ord-42"}, {"amount", "4200"}});

// stdout + rotating file, per-instance log level
auto log2 = JsonLogger::with_file("svc", "/var/log/svc.log");
log2.set_level(spdlog::level::warn);

// Correlation context auto-injected in all log lines within scope
CorrelationScope scope{CorrelationContext::generate()};
log.info("processing");  // → {"msg":"processing","cid":"...","tid":"...","rid":"..."}

// Prometheus text-format
PrometheusMetrics m;
m.increment("http_requests_total");
std::string body = m.render();  // serve on /metrics
```

### Resilience

```cpp
#include <retry_policy.hpp>
#include <circuit_breaker.hpp>
#include <hedge.hpp>
#include <rate_limiter.hpp>

// Exponential backoff + full jitter + typed filter
auto result = with_retry(
    {.max_attempts = 3, .initial_delay = 100ms, .should_retry = retry_on<NetworkError>()},
    [&] { return remote_call(); });

// Hedge: fire two requests, return faster one
auto value = hedge(50ms, [](std::atomic<bool>& cancelled) {
    return remote_call(cancelled);  // poll cancelled for early exit
});

// Circuit breaker: open after 5 failures, retry after 30s
CircuitBreaker cb{{.failure_threshold = 5, .open_duration = 30s}};
cb.call([&] { return db.query(sql); });
```

### Application (CQRS + Saga)

```cpp
#include <command_bus.hpp>
#include <query_bus.hpp>
#include <saga_orchestrator.hpp>

// CQRS
CommandBus bus;
bus.register_handler<PlaceOrderCmd>([](const auto& cmd) { /* ... */ });
bus.send(PlaceOrderCmd{"item-1", 3});

// Saga: steps run in sequence; failure triggers reverse compensation
SagaOrchestrator saga;
saga.step("reserve",
    [&]{ inventory.reserve(id, qty); },
    [&]{ inventory.release(id, qty); });  // compensation
saga.step("charge",
    [&]{ payment.charge(card, amt); },
    [&]{ payment.refund(card, amt); });
saga.step("confirm", [&]{ order.confirm(); });
saga.run();
```

### Web (HTTP middleware)

```cpp
#include <middleware_chain.hpp>
#include <cors_middleware.hpp>
#include <body_limit_middleware.hpp>
#include <compression_middleware.hpp>

MiddlewareChain chain;
chain.use(cors_middleware({.allowed_origins = {"https://app.example.com"}}));
chain.use(body_limit_middleware(1 * 1024 * 1024));  // 413 on > 1 MB
chain.use(compression_middleware());                  // gzip if Accept-Encoding: gzip
chain.use(auth_middleware([](const std::string& t) { return jwt.verify(t); }));

auto resp = chain.dispatch(req, final_handler);
```

### Security

```cpp
#include <jwt_signer.hpp>
#include <argon2_hasher.hpp>
#include <pii_redactor.hpp>

// JWT HS256
JwtSigner signer{"secret"};
auto token  = signer.sign({{"sub", "u-1"}, {"role", "admin"}});
auto claims = signer.verify(token);

// Argon2id (OWASP 2023 defaults: t=3, m=64MB, p=4)
Argon2Hasher hasher;
auto encoded = hasher.hash("hunter2");
bool ok      = hasher.verify(encoded, "hunter2");

// PII redaction for safe logging
auto safe = PiiRedactor::redact_all("user@example.com 4111-1111-1111-1111");
// → "[EMAIL] [CARD]"
```

### Config

```cpp
#include <settings.hpp>
#include <config_watcher.hpp>

// Duration suffix parser: "500ms", "10s", "2m", "1h"
auto timeout = env_as<std::chrono::milliseconds>("TIMEOUT");

// Collect all errors before throwing
ConfigValidator v;
auto host = v.require("DB_HOST");
auto port = v.require_int("DB_PORT");
v.check();  // throws with full error list

// Live reload on IN_CLOSE_WRITE (Linux)
ConfigWatcher watcher{"/etc/app/config.yaml", [&] { reload(); }};
watcher.start();
```

---

## Testing Fakes

All ports have in-memory test doubles under `include/cpp_commons/testing/`:

```cpp
#include <cpp_commons/testing/fake_logger.hpp>
#include <cpp_commons/testing/spy_metrics.hpp>
#include <cpp_commons/testing/fake_tracer.hpp>
#include <cpp_commons/testing/fake_repository.hpp>
#include <cpp_commons/testing/in_memory_event_bus.hpp>

FakeLogger logger;
logger.info("hello");
EXPECT_LOG_CONTAINS(logger, "hello");
```

---

## Build Presets

| Preset | Use |
|--------|-----|
| `dev` | Local dev — debug, ccache, `compile_commands.json` |
| `ci` | CI — UBSan + `-Werror` |
| `ci-asan` | CI — AddressSanitizer |
| `coverage` | gcov (`make coverage`) |
| `benchmark` | Release build + benchmarks |
| `release` | Optimised install artifact |

---

## CI

On every push/PR to `main`:

| Job | Description |
|-----|-------------|
| `build-and-test` | GCC-13 + Clang-17 × `ci` + `ci-asan` (Linux) |
| `build-and-test-macos` | Apple Clang on `macos-latest` |
| `format` | `clang-format-17 --dry-run --Werror` |
| `tidy` | `clang-tidy-17` on `include/` + `src/` |
| `coverage` | lcov with 80% line threshold |
| `benchmark` | JSON artifact per commit; fails on >20% regression |

---

## Architecture

Key decisions in [`docs/adr/`](docs/adr/):

- **ADR-001** — FetchContent over vcpkg for reproducibility
- **ADR-002** — Header-only kernel for template/concept portability
- **ADR-003** — `tl::expected` for GCC 12 compatibility
- **ADR-004** — OpenSSL behind `CPP_COMMONS_HAS_OPENSSL` guard

---

## License

MIT
