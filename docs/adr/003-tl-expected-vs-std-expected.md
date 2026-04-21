# ADR-003: tl::expected over std::expected

**Status:** Accepted  
**Date:** 2026-01-15

## Context

`Result<T,E>` needs an underlying sum type. Two candidates:

- **`std::expected<T,E>`** (C++23): standard library, no dependency.
- **`tl::expected`** (header-only, C++14+): backport by Sy Brand, widely adopted, production-tested.

## Decision

Wrap `tl::expected` internally. `Result<T,E>` exposes only its own API — callers never see `tl::expected` directly.

## Rationale

1. **Compiler support.** GCC 12 and Clang 16 ship incomplete `std::expected`. GCC 13 is the minimum guaranteed-complete implementation. Our CI matrix includes GCC 12 (TASKS item), which `std::expected` may not fully support.
2. **Richer monadic API.** `tl::expected` provides `map`, `map_error`, `and_then`, `or_else` on older standards. `std::expected` gained monadic ops only in P2505R5 (C++23), not widely available.
3. **Migration path.** Because `Result<T,E>` wraps the implementation, migrating from `tl::expected` to `std::expected` requires changing only `result.hpp` — zero consumer changes.
4. **Battle-tested.** `tl::expected` is used in production by hundreds of projects and has a stable API.

## Trade-offs

- Adds a FetchContent dependency (`TartanLlama/expected`). Small: ~700 lines, header-only.
- Consumers cannot pass `Result<T,E>` to APIs expecting `std::expected` (different types). Acceptable since `Result` is a domain type, not passed to stdlib.

## Consequences

- `include/cpp_commons/kernel/result.hpp` includes `<tl/expected.hpp>` and wraps it.
- When the project drops GCC 12 support and targets C++23, `tl::expected` can be swapped for `std::expected` in a single-file change.
