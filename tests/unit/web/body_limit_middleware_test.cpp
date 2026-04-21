#include <body_limit_middleware.hpp>
#include <middleware_chain.hpp>
#include <gtest/gtest.h>

using namespace cpp_commons::web;

static HttpResponse dispatch_with_limit(std::size_t limit, const HttpRequest& req) {
    MiddlewareChain chain;
    chain.use(body_limit_middleware(limit));
    Handler final_handler = [](const HttpRequest&) { return HttpResponse{200, {}, "ok"}; };
    return chain.dispatch(req, final_handler);
}

TEST(BodyLimitMiddlewareTest, AllowsBodyUnderLimit) {
    HttpRequest req;
    req.body = "hello";
    req.headers["Content-Length"] = "5";
    auto resp = dispatch_with_limit(100, req);
    EXPECT_EQ(resp.status_code, 200);
}

TEST(BodyLimitMiddlewareTest, RejectsContentLengthOverLimit) {
    HttpRequest req;
    req.headers["Content-Length"] = "1000";
    req.body = "";
    auto resp = dispatch_with_limit(100, req);
    EXPECT_EQ(resp.status_code, 413);
}

TEST(BodyLimitMiddlewareTest, RejectsBodyOverLimitWithoutHeader) {
    HttpRequest req;
    req.body = std::string(200, 'x');
    auto resp = dispatch_with_limit(100, req);
    EXPECT_EQ(resp.status_code, 413);
}

TEST(BodyLimitMiddlewareTest, AllowsBodyExactlyAtLimit) {
    HttpRequest req;
    req.body = std::string(100, 'x');
    req.headers["Content-Length"] = "100";
    auto resp = dispatch_with_limit(100, req);
    EXPECT_EQ(resp.status_code, 200);
}

TEST(BodyLimitMiddlewareTest, IgnoresMalformedContentLength) {
    HttpRequest req;
    req.headers["Content-Length"] = "not-a-number";
    req.body = "short";
    auto resp = dispatch_with_limit(100, req);
    EXPECT_EQ(resp.status_code, 200);
}

TEST(BodyLimitMiddlewareTest, ResponseIs413WithProblemJson) {
    HttpRequest req;
    req.headers["Content-Length"] = "999";
    auto resp = dispatch_with_limit(100, req);
    EXPECT_EQ(resp.status_code, 413);
    EXPECT_NE(resp.body.find("413"), std::string::npos);
}
