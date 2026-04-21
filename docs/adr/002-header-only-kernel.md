# ADR-002: Header-only kernel, compiled adapters

**Status:** Accepted  
**Date:** 2026-01-15

## Context

The library has two layers:

- **Kernel** (`include/cpp_commons/kernel/`): domain primitives — `Result`, `Option`, `UUID`, `ValueObject`, `Specification`, ports/concepts. Consumed by every module.
- **Adapters** (`src/`): concrete implementations — `JsonLogger`, `PrometheusMetrics`, `OtelTracer`, resilience policies, HTTP middleware. Depend on spdlog, nlohmann_json, OpenSSL, etc.

Should kernel types be header-only or compiled into a static library?

## Decision

Kernel is **header-only** (templates + inline functions, installed under `include/`).  
Adapters are **compiled** into named static libraries (`cpp_commons::observability`, `cpp_commons::web`, etc.).

## Rationale

1. **Zero-cost kernel adoption.** A consumer who only needs `Result<T,E>` or `UUID` links nothing — no spdlog, no OpenSSL, no yaml-cpp. One `target_include_directories` call suffices.
2. **Template requirements.** `Result<T,E>`, `Option<T>`, and `Specification<T>` are parameterised on user types. They must be header-only to instantiate at the call site.
3. **Concept-based ports.** `kernel::LoggerPort`, `MetricsPort`, and `TracerPort` are concepts — zero-overhead duck typing. Concepts cannot be pre-compiled.
4. **Adapter isolation.** Adapters carry heavy dependencies. Compiled libraries allow consumers to opt-in only to what they need (`cpp_commons::web` without `cpp_commons::security`).

## Trade-offs

- Heavy header-only use increases per-TU compile time if many translation units include kernel headers. Mitigated by modern include-what-you-use discipline and PCH if needed.
- Adapter compilation must be repeated per target (no shared library option currently).

## Consequences

- `CMakeLists.txt` defines `INTERFACE` library `cpp_commons::kernel` and separate `STATIC` libraries per adapter module.
- Kernel headers are installed to `<prefix>/include/cpp_commons/kernel/`.
- Adapters expose only their public API via `target_include_directories(... PUBLIC ...)`.
