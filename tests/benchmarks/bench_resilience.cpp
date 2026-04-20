#include <benchmark/benchmark.h>
#include <circuit_breaker.hpp>
#include <retry_policy.hpp>

using namespace cpp_commons::resilience;

// ── CircuitBreaker::call (Closed state, no contention) ───────────────────────

static void BM_CircuitBreakerClosedCall(benchmark::State& state) {
    CircuitBreaker cb{CircuitBreakerConfig{.failure_threshold = 1000}};
    for (auto _ : state) {
        auto r = cb.call([] { return 42; });
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(BM_CircuitBreakerClosedCall);

// ── Bulkhead acquire/release ─────────────────────────────────────────────────

#include <bulkhead.hpp>

static void BM_BulkheadCall(benchmark::State& state) {
    Bulkhead bh{64};
    for (auto _ : state) {
        auto r = bh.call([] { return 1; });
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(BM_BulkheadCall);
