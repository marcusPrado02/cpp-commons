# Contributing to cpp-commons

## Development Setup

```bash
# Clone and configure
git clone <repo>
cd cpp-commons
cmake --preset dev
cmake --build build/dev
ctest --preset dev
```

A VS Code devcontainer is provided (`.devcontainer/`) — open in VS Code and click "Reopen in Container".

## Code Style

- C++20, clang-format enforced (`make format-check`)
- clang-tidy gate on CI (`make lint`)
- All new code must compile clean under `-Werror -Wall -Wextra -Wsign-conversion`

Run before pushing:
```bash
make format
make lint
make test
```

## Testing

- Every new feature needs unit tests in `tests/unit/<module>/`
- Integration tests live in `tests/integration/`
- Aim for ≥80% line coverage (`make coverage`)
- Tests must pass under AddressSanitizer (`cmake --preset ci-asan && cmake --build --preset ci-asan && ctest --preset ci-asan`)

### Test Structure

```cpp
#include <gtest/gtest.h>
#include <your_header.hpp>

TEST(YourModuleTest, DescriptiveTestName) {
    // Arrange
    // Act
    // Assert
}
```

Use the fakes in `include/cpp_commons/testing/` instead of mocking production types.

## Design Principles

**YAGNI** — implement what is needed, not what might be needed.

**Ports and Adapters** — kernel types depend on concepts (`LoggerPort`, `MetricsPort`, etc.), not concrete implementations. Adapters satisfy the concept.

**Value Semantics** — prefer `ValueObject<Derived>` for domain primitives. Use `Result<T,E>` instead of exceptions for recoverable errors.

**No Hidden Dependencies** — headers must include what they use. No transitive include reliance.

## Pull Request Checklist

- [ ] `make format` shows no diff
- [ ] `make lint` passes
- [ ] `make test` passes
- [ ] New public API has a unit test
- [ ] No new raw owning pointers (`new`/`delete`) — use smart pointers or value types
- [ ] New `Result`-returning functions tested for both ok and err paths
- [ ] Thread-safety documented in comments if relevant

## Commit Messages

Follow conventional commits:

```
feat(module): short description

Body explaining why, not what. 72 char line limit.
```

Prefixes: `feat`, `fix`, `test`, `docs`, `refactor`, `perf`, `chore`.

## Adding a New Module

1. Create `src/<module>/` with implementation
2. Create `include/cpp_commons/<module>/` for public API (if needed)
3. Add `CMakeLists.txt` in `src/<module>/` following existing patterns
4. Add `add_subdirectory(src/<module>)` in root `CMakeLists.txt`
5. Add tests in `tests/unit/<module>/`
6. Export the library target as `cpp_commons::<module>`
