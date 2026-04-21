# Changelog

All notable changes to cpp-commons follow [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

### Added
- `HttpResponse::from_error()` — automatic dispatch from domain error hierarchy to HTTP status codes
- `cors_middleware()` — W3C CORS with wildcard/allowlist origins, preflight, and credentials
- `require_validated<T>()` — config predicate validation with constraint description
- `YamlSource` — YAML file/string config source backed by yaml-cpp (FetchContent)
- `EnvSource(prefix)` — 12-factor namespace prefix for environment variables
- `InMemoryEventBus` made thread-safe; concurrency tests added
- `Specification<T>::to_string()` — human-readable rejection expression for logging
- `ConfigValidator` — accumulates all config errors before throwing
- `env_as<std::chrono::milliseconds>()` with `parse_duration()` suffix parser (ms/s/m/h)
- `Deadline` + `RetryConfig::deadline` for deadline propagation
- `RateLimiter` token-bucket, `RateLimitMiddleware` (429 + Retry-After)
- `HealthHttpHandler` — `/health/live` and `/health/ready` endpoints
- `OutboxPublisher<EventBus>` — at-least-once domain event publishing
- `InstrumentedUseCase<In,Out,Err,Metrics>` — automatic metrics decoration
- `KeyRotationService` — versioned AES-GCM key rotation with 4-byte version prefix
- `CircuitBreaker` concurrency tests (8 threads, TSan-safe)
- `TracingMiddleware` — OpenTelemetry span per command/query
- `FakeHttpServer` — in-memory test double for middleware chain testing
- ADRs 001–004 documenting key architecture decisions
- ccache in `dev` CMake preset
- `cpp_commons.pc.in` pkg-config file for non-CMake consumers
- Makefile `install` and `fuzz` targets
- Adversarial tests for `JwtDecoder` (alg:none, oversized, malformed) and `PiiRedactor`
- `JsonLogger::with_file()` — multi-sink logger (stdout + rotating file); `set_level()` per instance
- `body_limit_middleware()` — 413 on `Content-Length` or body exceeding configured limit
- `compression_middleware()` — gzip response body when `Accept-Encoding: gzip` (zlib, optional)
- `ConfigWatcher` — inotify-based hot reload on Linux; callback on `IN_CLOSE_WRITE`
- `hedge()` — optimistic parallelism: fire second request after delay, return first winner
- `SagaOrchestrator` — sequential steps with named compensations executed in reverse on failure
- `Argon2Hasher` — argon2id password hashing (phc-winner-argon2 via FetchContent; OWASP defaults)
- CI: macOS job (Apple Clang on `macos-latest`)
- CI: benchmark artifact saved per commit; Python comparison script fails on >20% regression

---

## [0.1.0] — 2026-01-15

### Added
- `Result<T,E>` — railway-oriented error handling wrapping `tl::expected`. Methods: `map`, `map_err`, `and_then`, `flatten`.
- `Option<T>` — functional optional with `map`, `filter`, `value_or_else`.
- `UUID` — RFC 4122 v4 with `from_string()` parser, `std::hash` specialization.
- `StrongId<Tag>` — tag-dispatched identifier with `std::hash`.
- `ValueObject<Derived>` — CRTP base with equality/hash via `fields()` tuple.
- `Money` — ISO 4217 currency + int64 cents, overflow-safe arithmetic.
- `Email::parse()` / `PhoneNumber::parse()` — validated value objects.
- `Specification<T>` — composable predicates (`&&`, `||`, `!`).
- `DomainEvent`, `AggregateRoot<T>`, `Entity<Id>` domain primitives.
- `JsonLogger` — spdlog-backed structured JSON logger with correlation context injection.
- `CorrelationContext` — thread-local `cid/tid/rid` + W3C `traceparent`/`tracestate`.
- `PrometheusMetrics` — in-process Prometheus text-format adapter.
- `OtelTracer` — W3C Trace Context compatible span recorder.
- `HealthRegistry` + `HealthHttpHandler` — `/health/live` and `/health/ready`.
- `CircuitBreaker` — closed/open/half-open state machine (thread-safe).
- `RetryPolicy` with exponential backoff, Full/Equal jitter, `retry_on<E>()` filter.
- `Bulkhead` — concurrency limiter.
- `CommandBus`, `QueryBus`, `Mediator` — type-erased synchronous message bus.
- `UseCase<In,Out,Err>` base with `Result` return.
- `JwtSigner`/`JwtDecoder` — HMAC-SHA256 JWT sign and decode.
- `PiiRedactor` — regex-based email and credit card redaction.
- `AesGcmProvider` — AES-256-GCM encryption (OpenSSL, optional).
- `SecureRandom` — cryptographically secure token generation.
- HTTP middleware: auth, CORS, content negotiation, idempotency, rate limit, health.
- `FakeTracer`, `FakeLogger`, `SpyMetrics`, `FakeRepository`, `InMemoryEventBus`, `FakeHttpServer` test doubles.
- Custom GTest matchers: `IsOk`, `IsErr`, `IsOkWith`, `IsSome`, `IsNone`, `IsSomeWith`.
- GitHub Actions CI: GCC 13 + Clang 17, ASan, UBSan, clang-tidy, clang-format, 80% coverage gate.
- `.devcontainer/devcontainer.json` for one-click VS Code / Codespaces setup.

[Unreleased]: https://github.com/example/cpp-commons/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/example/cpp-commons/releases/tag/v0.1.0
