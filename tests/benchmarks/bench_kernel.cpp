#include <benchmark/benchmark.h>
#include <cpp_commons/kernel/identity.hpp>
#include <cpp_commons/kernel/result.hpp>
#include <cpp_commons/kernel/option.hpp>
#include <cpp_commons/errors/domain_error.hpp>

using namespace cpp_commons::kernel;
using namespace cpp_commons::errors;

// ── UUID::generate ────────────────────────────────────────────────────────────

static void BM_UuidGenerate(benchmark::State& state) {
    for (auto _ : state) {
        auto id = UUID::generate();
        benchmark::DoNotOptimize(id);
    }
}
BENCHMARK(BM_UuidGenerate);

static void BM_UuidToString(benchmark::State& state) {
    auto id = UUID::generate();
    for (auto _ : state) {
        auto s = id.to_string();
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(BM_UuidToString);

// ── Result<T,E> ───────────────────────────────────────────────────────────────

static void BM_ResultOkConstruct(benchmark::State& state) {
    for (auto _ : state) {
        auto r = Result<int, DomainError>::ok(42);
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(BM_ResultOkConstruct);

static void BM_ResultAndThen(benchmark::State& state) {
    auto r = Result<int, DomainError>::ok(1);
    for (auto _ : state) {
        auto r2 = r.and_then([](int v) {
            return tl::expected<int, DomainError>{v + 1};
        });
        benchmark::DoNotOptimize(r2);
    }
}
BENCHMARK(BM_ResultAndThen);

// ── Option<T> ─────────────────────────────────────────────────────────────────

static void BM_OptionSome(benchmark::State& state) {
    for (auto _ : state) {
        auto o = Option<int>::some(7);
        benchmark::DoNotOptimize(o);
    }
}
BENCHMARK(BM_OptionSome);
