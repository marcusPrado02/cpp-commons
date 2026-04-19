# cpp-commons Design Spec
**Date:** 2026-04-19  
**Author:** Marcus Prado Silva  
**Status:** Approved

---

## 1. Problem Statement

The project already has production-grade commons libraries for TypeScript (`ts-commons`, 66 packages) and Python (`python-commons`, single package with deep submodules). Both implement 12-factor app principles, DDD/Hexagonal architecture, CQRS, resilience patterns, and structured observability.

`cpp-commons` provides the equivalent golden-path building blocks for C++ services: a set of composable, well-bounded modules that encode senior-engineer patterns so individual service teams don't reinvent them.

---

## 2. Goals

- Mirror the architectural principles of `ts-commons` and `python-commons` for C++ services
- C++20 standard (Concepts, Coroutines, Ranges, `std::span`, `std::format`)
- CMake 3.25+ with vcpkg manifest mode for dependency management
- Single CMake project with multiple `cpp_commons::<module>` targets (Abseil/Folly model)
- Async via Asio standalone coroutines (`asio::awaitable<T>`)
- Abordagem A: header-only kernel + compiled adapters
- Scope B: Core modules + light adapters (no external services required to compile)

---

## 3. Non-Goals

- Heavy infrastructure adapters (Kafka, Redis, PostgreSQL drivers) — future optional modules
- C++20 Named Modules — deferred until toolchain support stabilizes (GCC 14+/Clang 17+)
- Bazel build system support in v1
- ABI stability guarantees in v1 (semver covers source compatibility)

---

## 4. Architecture

### 4.1 Layer Model

```
┌──────────────────────────────────────────────────────┐
│  consumers (application services)                    │
└────────────────────┬─────────────────────────────────┘
                     │ target_link_libraries
┌────────────────────▼─────────────────────────────────┐
│  ADAPTERS (compiled .a)                              │
│  application │ web │ security                        │
└──────┬───────┴─────┴──────────────────────────────────┘
       │
┌──────▼──────────────────────────────────────────────┐
│  INFRASTRUCTURE (compiled .a)                       │
│  config │ observability │ resilience                 │
└──────┬──────────────────────────────────────────────┘
       │
┌──────▼──────────────────────────────────────────────┐
│  KERNEL + ERRORS + TESTING  (header-only .hpp)      │
│  Result<T,E> │ Option<T> │ Entity │ AggregateRoot   │
│  ValueObject │ DomainEvent │ Ports │ ProblemDetails  │
└─────────────────────────────────────────────────────┘
```

**Dependency rule:** no layer imports from above. `kernel` has zero external dependencies (stdlib only).

### 4.2 CMake Target Graph

```
cpp_commons::kernel        (INTERFACE — header-only)
cpp_commons::errors        (INTERFACE — header-only, depends: kernel)
cpp_commons::testing       (INTERFACE — header-only, depends: kernel, errors, gtest)
cpp_commons::config        (STATIC,    depends: kernel, errors, nlohmann_json)
cpp_commons::observability (STATIC,    depends: kernel, errors, config, spdlog, asio)
cpp_commons::resilience    (STATIC,    depends: kernel, errors, observability, asio)
cpp_commons::application   (STATIC,    depends: kernel, errors, observability, resilience)
cpp_commons::web           (STATIC,    depends: kernel, errors, application, nlohmann_json)
cpp_commons::security      (STATIC,    depends: kernel, errors, openssl, jwt-cpp)
```

---

## 5. Module Inventory

### 5.1 `kernel` (header-only)

**Location:** `include/cpp_commons/kernel/`  
**External deps:** none (stdlib C++20 only)

| File | Export |
|------|--------|
| `result.hpp` | `Result<T,E>` — railway-oriented via `tl::expected` |
| `option.hpp` | `Option<T>` — semantic wrapper over `std::optional` |
| `entity.hpp` | `Entity<TId>` — CRTP, identity by ID, C++20 Concept constraint |
| `aggregate_root.hpp` | `AggregateRoot<TId>` — records/pulls domain events |
| `value_object.hpp` | `ValueObject<T>` — CRTP, equality by value, immutable |
| `domain_event.hpp` | `DomainEvent` — base with auto-populated metadata |
| `specification.hpp` | `Specification<T>` — composable `&&`, `\|\|`, `!` |
| `clock.hpp` | `Clock` concept + `SystemClock` + `FrozenClock` |
| `identity.hpp` | `EntityId`, `CorrelationId`, `TenantId`, `UUID` |
| `ports/logger_port.hpp` | `LoggerPort` — C++20 concept (not inheritance) |
| `ports/metrics_port.hpp` | `MetricsPort` — C++20 concept |
| `ports/tracer_port.hpp` | `TracerPort` — C++20 concept |
| `ports/repository_port.hpp` | `RepositoryPort<T,TId>` — C++20 concept |

**Key design decisions:**
- Ports are C++20 Concepts, not abstract base classes. This enables static polymorphism and zero-cost abstractions at the kernel boundary without vtable overhead.
- `Result<T,E>` wraps `tl::expected` with ergonomic `and_then`/`or_else`/`map` methods matching the ts-commons API surface. Migrates to `std::expected` (C++23) without consumer changes.
- `AggregateRoot` records events via `record(DomainEvent&&)` and drains them via `pull_events()` — the transactional outbox pattern at the domain level.

### 5.2 `errors` (header-only)

**Location:** `include/cpp_commons/errors/`

```
DomainError
  ├── NotFoundError
  ├── ConflictError
  └── InvariantViolationError
ApplicationError
  ├── ValidationError
  ├── UnauthorizedError
  ├── ForbiddenError
  ├── RateLimitError
  └── TimeoutError
InfrastructureError
  └── ExternalServiceError
```

`ProblemDetails` implements RFC 7807 — serializable to JSON for HTTP error responses.

### 5.3 `config` (compiled)

**Location:** `src/config/`  
**External deps:** `nlohmann-json`

- `Settings` base CRTP class — typed env var parsing with `require_env<T>()`, `env_or<T>()`, `env_flag()`, `env_duration()`
- `ConfigLoader::load<T>()` — validates all required vars at startup, throws `ConfigError` (never at request time)
- `EnvSource` — reads `::getenv()`
- `DotenvSource` — parses `.env` files (development only)
- `ConfigError` — aggregates all missing/invalid vars in one throw

### 5.4 `observability` (compiled)

**Location:** `src/observability/`  
**External deps:** `spdlog`, `asio`, `nlohmann-json`

- `JsonLoggerFactory::create(level)` — structured JSON sink via spdlog, auto-injects `correlation_id` from context
- `CorrelationContext` — propagated via `ContextAwareExecutor` wrapping Asio's executor; survives coroutine suspension across thread-pool threads (no `thread_local`)
- `MetricsPort` concept + `PrometheusMetrics` implementation
- `HealthRegistry` — registers named checks, exposes `/health/live` and `/health/ready`
- `SloTracker` — SLI/SLO tracking primitives

### 5.5 `resilience` (compiled + Asio coroutines)

**Location:** `src/resilience/`  
**External deps:** `asio`

- `RetryPolicy` — builder pattern, `max_attempts`, `ExponentialBackoff`, `LinearBackoff`, `FullJitter`/`DecorrelatedJitter`, `retry_on<E>()`. Uses `co_await asio::steady_timer` (non-blocking backoff)
- `CircuitBreaker` — Closed/Open/Half-open state machine, configurable thresholds
- `Bulkhead` — semaphore-based concurrency limiter per resource
- `Timeout` — `co_await with_timeout(duration, awaitable)`
- `Deadline` — `DeadlineContext` propagated through coroutine chain

### 5.6 `application` (compiled)

**Location:** `src/application/`

- `UseCase<TInput, TOutput, TError>` — base interface
- `CommandBus` — builder with middleware chain, handler registration
- `QueryBus` — same model as CommandBus
- `Mediator` — dispatches commands and queries
- Built-in middlewares: `LoggingMiddleware`, `ValidationMiddleware`, `TracingMiddleware`, `IdempotencyMiddleware`
- `PageRequest` / `Page<T>` — offset and cursor pagination primitives

### 5.7 `web` (compiled)

**Location:** `src/web/`  
**External deps:** `nlohmann-json`

- `HttpRequest` / `HttpResponse` — framework-agnostic abstractions
- `MiddlewareChain` — composable request pipeline
- Built-in middlewares: `CorrelationMiddleware`, `AuthMiddleware`, `RequestLoggingMiddleware`
- `HttpResponse::from_error(DomainError)` → maps to `ProblemDetails` + correct HTTP status

### 5.8 `security` (compiled)

**Location:** `src/security/`  
**External deps:** `openssl`, `jwt-cpp`

- `JwtDecoder` — JWKS-backed token verification, throws `UnauthorizedError`
- `AesGcmProvider` — AES-256-GCM encrypt/decrypt via OpenSSL 3
- `KeyRotationService` — manages key versions for rolling rotations
- `ApiKeyGenerator::generate()` → `(raw_key, bcrypt_hash)` pair
- `ApiKeyVerifier::verify(raw, hash)` → `bool`
- `PiiRedactor` — field-name-based redaction for log sanitization

### 5.9 `testing` (header-only)

**Location:** `include/cpp_commons/testing/`  
**External deps:** `gtest`

- `FakeClock` — implements `Clock` concept, `advance(duration)`, `set(time_point)`
- `FakeLogger` — captures log calls, `EXPECT_THAT(logger.messages(), Contains(...))`
- `NoopMetrics` / `SpyMetrics` — implements `MetricsPort` concept
- `InMemoryEventBus` — `publish()` + `events_of<T>()`
- `Builder<T>` — fluent object builder for test fixtures

---

## 6. External Dependencies (vcpkg.json)

| Package | Version | Use |
|---------|---------|-----|
| `tl-expected` | `^1.1.0` | `Result<T,E>` |
| `asio` | `^1.28.0` | Async executor + coroutines |
| `spdlog` | `^1.13.0` | Structured logging |
| `nlohmann-json` | `^3.11.0` | JSON serialization |
| `openssl` | `^3.2.0` | AES-GCM, TLS |
| `jwt-cpp` | `^0.7.0` | JWT decode/verify |
| `gtest` | `^1.14.0` | Unit testing |
| `benchmark` | `^1.8.0` | Microbenchmarks |
| `fmt` | `^10.0.0` | String formatting (spdlog dep) |
| `abseil` | `^20240116` | UUID, string utilities |

---

## 7. Build System

### 7.1 CMake Presets

| Preset | Build Type | Sanitizers | clang-tidy | Tests |
|--------|-----------|-----------|-----------|-------|
| `dev` | Debug | ASan + UBSan | ON | ON |
| `ci` | Release | UBSan | OFF | ON |
| `release` | Release | OFF | OFF | OFF |
| `benchmark` | Release | OFF | OFF | OFF |

### 7.2 Compiler Flags

All targets inherit a `cpp_commons_warnings` INTERFACE target with:
`-Wall -Wextra -Wpedantic -Werror -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wconversion -Wsign-conversion -Wnull-dereference`

### 7.3 Quality Gates

| Gate | Tool | Threshold |
|------|------|-----------|
| Warnings | GCC/Clang `-Werror` | 0 warnings |
| Formatting | `clang-format` Google style | CI fails on diff |
| Static analysis | `clang-tidy` modernize + bugprone + cppcoreguidelines | CI fails on findings |
| Unit tests | GoogleTest + CTest | 100% pass |
| Coverage | gcov + lcov | ≥ 80% line coverage |
| Sanitizers (dev) | ASan + UBSan | 0 errors |
| Sanitizers (ci) | UBSan | 0 errors |

---

## 8. Async & Concurrency Model

- All async operations return `asio::awaitable<T>`
- `io_context` is owned by the consuming service, not by cpp-commons
- `CorrelationContext` is propagated via `ContextAwareExecutor` — a thin executor adaptor that injects context before each coroutine resume. This survives thread-pool migration, unlike `thread_local`
- Non-blocking backoff in `RetryPolicy` via `co_await asio::steady_timer` — thread returns to pool during wait
- No blocking calls (`std::this_thread::sleep_for`, blocking sockets) anywhere in request paths

---

## 9. Error Handling Strategy

- **Domain logic:** `Result<T,E>` — no exceptions in the happy path
- **Startup/configuration:** `ConfigError` thrown — fail fast before serving traffic
- **Infrastructure boundaries:** exceptions caught at adapter boundaries, mapped to `Result` or `ProblemDetails`
- **Coroutines:** `co_return Result::err(...)` propagates errors without stack unwinding overhead

---

## 10. Data Flow (end-to-end)

```
HTTP Request
  → CorrelationMiddleware (extract/generate correlation_id, create ContextAwareExecutor)
  → AuthMiddleware (JwtDecoder::decode → Principal)
  → RequestLoggingMiddleware
  → CommandBus
      → LoggingMiddleware
      → ValidationMiddleware
      → TracingMiddleware
      → CreateOrderHandler::handle(cmd)
          → Order::create(amount, customer_id)    [records OrderCreated event]
          → co_await retry(repo.save(order))       [CircuitBreaker wrapped]
          → co_await event_bus.publish(events)
          → Result<OrderId, DomainError>::ok(id)
  → HttpResponse::created(order_id)               [201 + ProblemDetails on error]
```

---

## 11. Directory Structure

```
cpp-commons/
├── CMakeLists.txt
├── CMakePresets.json
├── vcpkg.json
├── vcpkg-configuration.json
├── Makefile
├── .clang-format
├── .clang-tidy
├── cmake/
│   ├── CppCommonsConfig.cmake.in
│   ├── CompilerWarnings.cmake
│   ├── Sanitizers.cmake
│   └── StaticAnalysis.cmake
├── include/cpp_commons/
│   ├── kernel/
│   │   ├── result.hpp
│   │   ├── option.hpp
│   │   ├── entity.hpp
│   │   ├── aggregate_root.hpp
│   │   ├── value_object.hpp
│   │   ├── domain_event.hpp
│   │   ├── specification.hpp
│   │   ├── clock.hpp
│   │   ├── identity.hpp
│   │   └── ports/
│   │       ├── logger_port.hpp
│   │       ├── metrics_port.hpp
│   │       ├── tracer_port.hpp
│   │       └── repository_port.hpp
│   ├── errors/
│   │   ├── domain_error.hpp
│   │   ├── error_codes.hpp
│   │   └── problem_details.hpp
│   └── testing/
│       ├── fake_clock.hpp
│       ├── fake_logger.hpp
│       ├── fake_metrics.hpp
│       └── builders.hpp
├── src/
│   ├── config/
│   ├── observability/
│   ├── resilience/
│   ├── application/
│   ├── web/
│   └── security/
├── tests/
│   ├── unit/
│   ├── integration/
│   └── benchmarks/
├── examples/
│   └── order_service/
└── docs/
    └── superpowers/specs/
        └── 2026-04-19-cpp-commons-design.md
```

---

## 12. Consuming the Library (golden path)

```cmake
# In consuming service's CMakeLists.txt
find_package(cpp_commons REQUIRED)

target_link_libraries(order_service PRIVATE
    cpp_commons::kernel
    cpp_commons::config
    cpp_commons::observability
    cpp_commons::resilience
    cpp_commons::application
)
```

Or via `FetchContent`:

```cmake
FetchContent_Declare(cpp_commons
    GIT_REPOSITORY https://github.com/marcusPrado02/cpp-commons.git
    GIT_TAG        v0.1.0
)
FetchContent_MakeAvailable(cpp_commons)
```

---

## 13. Future Modules (out of v1 scope)

- `cpp_commons::persistence` — repository adapters (libpq/PostgreSQL, SQLite)
- `cpp_commons::messaging_kafka` — Kafka producer/consumer (librdkafka)
- `cpp_commons::cache_redis` — Redis client (hiredis)
- `cpp_commons::grpc` — gRPC channel + health check integration
- `cpp_commons::otel` — OpenTelemetry SDK integration
