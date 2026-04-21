#include <cors_middleware.hpp>
#include <gtest/gtest.h>

using namespace cpp_commons::web;

static HttpResponse echo_ok(const HttpRequest&) {
    return {200, {}, "ok"};
}

TEST(CorsMiddlewareTest, WildcardOriginSetOnSimpleRequest) {
    auto mw = cors_middleware();
    HttpRequest req;
    req.method = HttpMethod::Get;
    req.path   = "/api";
    req.headers["Origin"] = "https://example.com";

    auto resp = mw(req, echo_ok);
    EXPECT_EQ(resp.headers.at("Access-Control-Allow-Origin"), "*");
}

TEST(CorsMiddlewareTest, AllowsListedOrigin) {
    CorsOptions opts;
    opts.allowed_origins = {"https://app.example.com"};
    auto mw = cors_middleware(opts);

    HttpRequest req;
    req.method = HttpMethod::Get;
    req.path   = "/";
    req.headers["Origin"] = "https://app.example.com";

    auto resp = mw(req, echo_ok);
    EXPECT_EQ(resp.headers.at("Access-Control-Allow-Origin"), "https://app.example.com");
}

TEST(CorsMiddlewareTest, BlocksUnlistedOrigin) {
    CorsOptions opts;
    opts.allowed_origins = {"https://trusted.example.com"};
    auto mw = cors_middleware(opts);

    HttpRequest req;
    req.method = HttpMethod::Get;
    req.path   = "/";
    req.headers["Origin"] = "https://evil.example.com";

    auto resp = mw(req, echo_ok);
    EXPECT_EQ(resp.headers.count("Access-Control-Allow-Origin"), 0u);
}

TEST(CorsMiddlewareTest, PreflightReturns204WithHeaders) {
    auto mw = cors_middleware();

    HttpRequest req;
    req.method = HttpMethod::Options;
    req.path   = "/api";
    req.headers["Origin"] = "https://example.com";

    auto resp = mw(req, echo_ok);
    EXPECT_EQ(resp.status_code, 204);
    EXPECT_NE(resp.headers.count("Access-Control-Allow-Methods"), 0u);
    EXPECT_NE(resp.headers.count("Access-Control-Allow-Headers"), 0u);
}

TEST(CorsMiddlewareTest, CredentialsHeaderSetWhenEnabled) {
    CorsOptions opts;
    opts.allow_credentials = true;
    auto mw = cors_middleware(opts);

    HttpRequest req;
    req.method = HttpMethod::Get;
    req.path   = "/";
    req.headers["Origin"] = "https://example.com";

    auto resp = mw(req, echo_ok);
    EXPECT_EQ(resp.headers.at("Access-Control-Allow-Credentials"), "true");
}
