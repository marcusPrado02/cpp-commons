#include <idempotency_middleware.hpp>
#include <middleware_chain.hpp>
#include <gtest/gtest.h>
#include <string>

using namespace cpp_commons::web;

TEST(IdempotencyMiddlewareTest, FirstRequestExecutesHandler) {
    int call_count = 0;
    MiddlewareChain chain;
    chain.use(idempotency_middleware());

    HttpRequest req;
    req.headers["Idempotency-Key"] = "key-1";
    (void)chain.dispatch(req, [&](const HttpRequest&) {
        ++call_count;
        return HttpResponse::ok("created");
    });

    EXPECT_EQ(call_count, 1);
}

TEST(IdempotencyMiddlewareTest, RepeatRequestReturnsCachedResponse) {
    int call_count = 0;
    MiddlewareChain chain;
    chain.use(idempotency_middleware());

    HttpRequest req;
    req.headers["Idempotency-Key"] = "key-dup";

    auto handler = [&](const HttpRequest&) -> HttpResponse {
        ++call_count;
        return HttpResponse::ok("original");
    };

    auto r1 = chain.dispatch(req, handler);
    auto r2 = chain.dispatch(req, handler);

    EXPECT_EQ(call_count, 1);
    EXPECT_EQ(r1.body, r2.body);
    EXPECT_EQ(r2.body, "original");
}

TEST(IdempotencyMiddlewareTest, NoKeyPassesThrough) {
    int call_count = 0;
    MiddlewareChain chain;
    chain.use(idempotency_middleware());

    HttpRequest req;  // no Idempotency-Key header

    (void)chain.dispatch(req, [&](const HttpRequest&) { ++call_count; return HttpResponse::ok(""); });
    (void)chain.dispatch(req, [&](const HttpRequest&) { ++call_count; return HttpResponse::ok(""); });

    EXPECT_EQ(call_count, 2);  // no caching without key
}

TEST(IdempotencyMiddlewareTest, DifferentKeysExecuteSeparately) {
    int call_count = 0;
    MiddlewareChain chain;
    chain.use(idempotency_middleware());

    auto handler = [&](const HttpRequest&) -> HttpResponse {
        ++call_count;
        return HttpResponse::ok("ok");
    };

    HttpRequest req1, req2;
    req1.headers["Idempotency-Key"] = "key-a";
    req2.headers["Idempotency-Key"] = "key-b";

    (void)chain.dispatch(req1, handler);
    (void)chain.dispatch(req2, handler);
    (void)chain.dispatch(req1, handler);  // cached
    (void)chain.dispatch(req2, handler);  // cached

    EXPECT_EQ(call_count, 2);
}

TEST(IdempotencyMiddlewareTest, CachedResponsePreservesStatusCode) {
    MiddlewareChain chain;
    chain.use(idempotency_middleware());

    HttpRequest req;
    req.headers["Idempotency-Key"] = "key-status";

    auto handler = [](const HttpRequest&) -> HttpResponse {
        return HttpResponse{201, {}, "created"};
    };

    (void)chain.dispatch(req, handler);
    auto cached = chain.dispatch(req, handler);

    EXPECT_EQ(cached.status_code, 201);
}
