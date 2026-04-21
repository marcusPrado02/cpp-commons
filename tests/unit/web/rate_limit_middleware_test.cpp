#include <rate_limit_middleware.hpp>
#include <middleware_chain.hpp>
#include <gtest/gtest.h>

using namespace cpp_commons::web;

TEST(RateLimitMiddlewareTest, AllowsRequestsWithinLimit) {
    MiddlewareChain chain;
    chain.use(rate_limit_middleware(3.0, 0.0));  // 3 tokens, no refill

    HttpRequest req;
    for (int i = 0; i < 3; ++i) {
        auto resp = chain.dispatch(req, [](const HttpRequest&) { return HttpResponse::ok("ok"); });
        EXPECT_EQ(resp.status_code, 200) << "Request " << i << " should be allowed";
    }
}

TEST(RateLimitMiddlewareTest, Returns429WhenExhausted) {
    MiddlewareChain chain;
    chain.use(rate_limit_middleware(2.0, 0.0));

    HttpRequest req;
    (void)chain.dispatch(req, [](const HttpRequest&) { return HttpResponse::ok("ok"); });
    (void)chain.dispatch(req, [](const HttpRequest&) { return HttpResponse::ok("ok"); });

    auto resp = chain.dispatch(req, [](const HttpRequest&) { return HttpResponse::ok("ok"); });
    EXPECT_EQ(resp.status_code, 429);
    EXPECT_EQ(resp.headers.at("Content-Type"), "application/problem+json");
}

TEST(RateLimitMiddlewareTest, RetryAfterHeaderPresent) {
    MiddlewareChain chain;
    chain.use(rate_limit_middleware(0.0, 0.0, "5"));  // 0 tokens → immediate 429

    HttpRequest req;
    auto resp = chain.dispatch(req, [](const HttpRequest&) { return HttpResponse::ok("ok"); });
    EXPECT_EQ(resp.status_code, 429);
    EXPECT_EQ(resp.headers.at("Retry-After"), "5");
}

TEST(RateLimitMiddlewareTest, HandlerNotCalledWhenRateLimited) {
    int calls = 0;
    MiddlewareChain chain;
    chain.use(rate_limit_middleware(0.0, 0.0));

    HttpRequest req;
    (void)chain.dispatch(req, [&](const HttpRequest&) { ++calls; return HttpResponse::ok(""); });
    EXPECT_EQ(calls, 0);
}
