# ADR-001: FetchContent over vcpkg for dependency management

**Status:** Accepted  
**Date:** 2026-01-15

## Context

cpp-commons is consumed as a FetchContent dependency by other CMake projects. We need a way to pull in third-party libraries (tl-expected, nlohmann_json, spdlog, googletest, yaml-cpp) without requiring the consumer to pre-install anything.

Two main options exist:

- **FetchContent** (CMake built-in): downloads and builds dependencies at configure time. No external toolchain required.
- **vcpkg**: package manager with a curated port registry. Requires vcpkg bootstrap and a toolchain file.

## Decision

Use CMake `FetchContent` with pinned `GIT_TAG` for every dependency.

## Rationale

1. **Zero bootstrap cost.** A consumer only needs CMake 3.28+, a C++20 compiler, and Ninja. No `vcpkg install`, no manifest file, no toolchain injection.
2. **FetchContent dependency reuse.** When a parent project already fetches spdlog or googletest, CMake deduplicates them. vcpkg would create a separate install tree.
3. **Hermetic CI.** The CI pipeline installs nothing beyond the compiler; all deps are fetched during `cmake --preset ci`.
4. **Pinned tags.** `GIT_TAG 0.6.3` etc. make builds reproducible without a lock file.

## Trade-offs

- Longer cold-configure time (deps fetched on first run, then cached in `build/_deps`).
- No pre-built binary cache (vcpkg can serve pre-compiled binaries on Azure Pipelines).
- Updating a dependency requires a manual `GIT_TAG` bump vs `vcpkg x-update-baseline`.

## Consequences

- All `find_package()` calls are replaced by `FetchContent_Declare` + `FetchContent_MakeAvailable`.
- `build/_deps` is gitignored and cached in CI via `actions/cache`.
- Consumers who use vcpkg can still add cpp-commons via FetchContent without conflict.
