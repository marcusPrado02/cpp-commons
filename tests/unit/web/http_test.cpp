#include <http_request.hpp>
#include <http_response.hpp>
#include <middleware_chain.hpp>
#include <gtest/gtest.h>

using namespace cpp_commons::web;

TEST(HttpRequestTest, HeaderLookup) {
    HttpRequest req;
    req.headers["Content-Type"] = "application/json";
    EXPECT_EQ(req.header("Content-Type").value_or(""), "application/json");
    EXPECT_FALSE(req.header("X-Missing").has_value());
}

TEST(HttpRequestTest, QueryParamLookup) {
    HttpRequest req;
    req.query_params["page"] = "2";
    EXPECT_EQ(req.param("page").value_or(""), "2");
    EXPECT_FALSE(req.param("limit").has_value());
}

TEST(HttpResponseTest, OkFactory) {
    auto resp = HttpResponse::ok(R"({"id":1})");
    EXPECT_EQ(resp.status_code, 200);
    EXPECT_EQ(resp.body, R"({"id":1})");
}

TEST(HttpResponseTest, NoContent) {
    auto resp = HttpResponse::no_content();
    EXPECT_EQ(resp.status_code, 204);
    EXPECT_TRUE(resp.body.empty());
}

TEST(MiddlewareChainTest, ExecutesFinalHandler) {
    MiddlewareChain chain;
    HttpRequest req;
    auto resp = chain.dispatch(req, [](const HttpRequest&) {
        return HttpResponse::ok("hello");
    });
    EXPECT_EQ(resp.status_code, 200);
    EXPECT_EQ(resp.body, "hello");
}

TEST(MiddlewareChainTest, MiddlewareRunsInOrder) {
    MiddlewareChain chain;
    std::string log;

    chain.use([&log](const HttpRequest& r, const Handler& next) {
        log += "A";
        auto resp = next(r);
        log += "A'";
        return resp;
    });
    chain.use([&log](const HttpRequest& r, const Handler& next) {
        log += "B";
        auto resp = next(r);
        log += "B'";
        return resp;
    });

    HttpRequest req;
    [[maybe_unused]] auto _ = chain.dispatch(req, [](const HttpRequest&) { return HttpResponse::ok(""); });
    EXPECT_EQ(log, "ABB'A'");
}

TEST(MiddlewareChainTest, MiddlewareCanShortCircuit) {
    MiddlewareChain chain;
    chain.use([](const HttpRequest&, const Handler&) {
        return HttpResponse{401, {}, "Unauthorized"};
    });

    HttpRequest req;
    auto resp = chain.dispatch(req, [](const HttpRequest&) {
        return HttpResponse::ok("should not reach");
    });
    EXPECT_EQ(resp.status_code, 401);
}
