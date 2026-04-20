#include <benchmark/benchmark.h>
#include <http_request.hpp>
#include <http_response.hpp>
#include <middleware_chain.hpp>

using namespace cpp_commons::web;

// ── MiddlewareChain dispatch (no middlewares, direct handler) ─────────────────

static void BM_ChainDirectDispatch(benchmark::State& state) {
    MiddlewareChain chain;
    HttpRequest req;
    req.path   = "/bench";
    req.method = HttpMethod::Get;

    for (auto _ : state) {
        auto resp = chain.dispatch(req, [](const HttpRequest&) {
            return HttpResponse::ok("{}");
        });
        benchmark::DoNotOptimize(resp);
    }
}
BENCHMARK(BM_ChainDirectDispatch);

// ── MiddlewareChain dispatch with 3 pass-through middlewares ──────────────────

static void BM_Chain3MiddlewareDispatch(benchmark::State& state) {
    MiddlewareChain chain;
    auto pass = [](const HttpRequest& r, const Handler& next) { return next(r); };
    chain.use(pass).use(pass).use(pass);

    HttpRequest req;
    req.path   = "/bench";
    req.method = HttpMethod::Get;

    for (auto _ : state) {
        auto resp = chain.dispatch(req, [](const HttpRequest&) {
            return HttpResponse::ok("{}");
        });
        benchmark::DoNotOptimize(resp);
    }
}
BENCHMARK(BM_Chain3MiddlewareDispatch);

// ── HttpRequest header lookup ─────────────────────────────────────────────────

static void BM_HeaderLookupHit(benchmark::State& state) {
    HttpRequest req;
    req.headers["Authorization"]  = "Bearer token123";
    req.headers["Content-Type"]   = "application/json";
    req.headers["X-Tenant-ID"]    = "tenant-acme";

    for (auto _ : state) {
        auto v = req.header("Authorization");
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(BM_HeaderLookupHit);
