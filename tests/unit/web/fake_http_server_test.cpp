#include <fake_http_server.hpp>
#include <rate_limit_middleware.hpp>
#include <content_negotiation.hpp>
#include <gtest/gtest.h>

using namespace cpp_commons::web;

TEST(FakeHttpServerTest, Returns404ForUnregisteredRoute) {
    FakeHttpServer server;
    auto resp = server.get("/missing");
    EXPECT_EQ(resp.status_code, 404);
}

TEST(FakeHttpServerTest, DispatchesRegisteredGetRoute) {
    FakeHttpServer server;
    server.route(HttpMethod::Get, "/hello", [](const HttpRequest&) {
        return HttpResponse::ok(R"({"msg":"hello"})");
    });

    auto resp = server.get("/hello");
    EXPECT_EQ(resp.status_code, 200);
    EXPECT_EQ(resp.body, R"({"msg":"hello"})");
}

TEST(FakeHttpServerTest, DispatchesRegisteredPostRoute) {
    FakeHttpServer server;
    server.route(HttpMethod::Post, "/echo", [](const HttpRequest& req) {
        return HttpResponse::ok(req.body);
    });

    auto resp = server.post("/echo", "payload");
    EXPECT_EQ(resp.status_code, 200);
    EXPECT_EQ(resp.body, "payload");
}

TEST(FakeHttpServerTest, MiddlewareRunsBeforeHandler) {
    FakeHttpServer server;
    server.use(rate_limit_middleware(0.0, 0.0));  // always exhausted
    server.route(HttpMethod::Get, "/blocked", [](const HttpRequest&) {
        return HttpResponse::ok("should not reach");
    });

    auto resp = server.get("/blocked");
    EXPECT_EQ(resp.status_code, 429);
}

TEST(FakeHttpServerTest, MultipleMiddlewaresChainInOrder) {
    std::vector<int> order;
    FakeHttpServer server;
    server.use([&](const HttpRequest& req, const Handler& next) {
        order.push_back(1);
        auto r = next(req);
        order.push_back(3);
        return r;
    });
    server.use([&](const HttpRequest& req, const Handler& next) {
        order.push_back(2);
        return next(req);
    });
    server.route(HttpMethod::Get, "/", [](const HttpRequest&) {
        return HttpResponse::ok("");
    });

    (void)server.get("/");
    EXPECT_EQ(order, (std::vector<int>{1, 2, 3}));
}

TEST(FakeHttpServerTest, GetPassesHeaders) {
    FakeHttpServer server;
    server.route(HttpMethod::Get, "/hdr", [](const HttpRequest& req) {
        return HttpResponse::ok(req.header("X-Custom").value_or("missing"));
    });

    auto resp = server.get("/hdr", {{"X-Custom", "hello"}});
    EXPECT_EQ(resp.body, "hello");
}

TEST(FakeHttpServerTest, ContentNegotiationMiddlewareIntegration) {
    FakeHttpServer server;
    server.use(content_negotiation_middleware({"application/json"}));
    server.route(HttpMethod::Get, "/data", [](const HttpRequest&) {
        return HttpResponse::ok(R"({"x":1})");
    });

    // Matching Accept
    auto ok = server.get("/data", {{"Accept", "application/json"}});
    EXPECT_EQ(ok.status_code, 200);
    EXPECT_EQ(ok.headers.at("Content-Type"), "application/json");

    // Non-matching Accept
    auto fail = server.get("/data", {{"Accept", "text/xml"}});
    EXPECT_EQ(fail.status_code, 406);
}
