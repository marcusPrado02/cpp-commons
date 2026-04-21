#include <content_negotiation.hpp>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace cpp_commons::web;

// ── negotiate_content_type ───────────────────────────────────────────────────

TEST(ContentNegotiationTest, ExactMatchReturnsType) {
    auto t = negotiate_content_type("application/json", {"application/json", "text/html"});
    EXPECT_EQ(t, "application/json");
}

TEST(ContentNegotiationTest, WildcardAcceptsFirst) {
    auto t = negotiate_content_type("*/*", {"text/html", "application/json"});
    EXPECT_EQ(t, "text/html");
}

TEST(ContentNegotiationTest, EmptyAcceptAcceptsFirst) {
    auto t = negotiate_content_type("", {"application/json"});
    EXPECT_EQ(t, "application/json");
}

TEST(ContentNegotiationTest, NoMatchReturnsEmpty) {
    auto t = negotiate_content_type("text/xml", {"application/json"});
    EXPECT_EQ(t, "");
}

TEST(ContentNegotiationTest, QualityFactorOrdering) {
    // Prefer text/html (q=0.9) over application/json (q=1.0) from the client's perspective,
    // but application/json has q=1.0 so it wins.
    auto t = negotiate_content_type(
        "text/html;q=0.9,application/json;q=1.0",
        {"text/html", "application/json"});
    EXPECT_EQ(t, "application/json");
}

TEST(ContentNegotiationTest, WildcardSubtypeMatches) {
    auto t = negotiate_content_type("application/*", {"application/json", "text/html"});
    EXPECT_EQ(t, "application/json");
}

// ── content_negotiation_middleware ───────────────────────────────────────────

TEST(ContentNegotiationMiddlewareTest, SetsContentTypeOnMatch) {
    MiddlewareChain chain;
    chain.use(content_negotiation_middleware({"application/json"}));

    HttpRequest req;
    req.headers["Accept"] = "application/json";

    auto resp = chain.dispatch(req, [](const HttpRequest&) {
        return HttpResponse::ok(R"({"ok":true})");
    });

    EXPECT_EQ(resp.status_code, 200);
    EXPECT_EQ(resp.headers.at("Content-Type"), "application/json");
}

TEST(ContentNegotiationMiddlewareTest, Returns406OnNoMatch) {
    MiddlewareChain chain;
    chain.use(content_negotiation_middleware({"application/json"}));

    HttpRequest req;
    req.headers["Accept"] = "text/xml";

    auto resp = chain.dispatch(req, [](const HttpRequest&) {
        return HttpResponse::ok("never");
    });

    EXPECT_EQ(resp.status_code, 406);
}

TEST(ContentNegotiationMiddlewareTest, NoAcceptHeaderPassesThrough) {
    MiddlewareChain chain;
    chain.use(content_negotiation_middleware({"application/json"}));

    HttpRequest req;  // no Accept header

    auto resp = chain.dispatch(req, [](const HttpRequest&) {
        return HttpResponse::ok("ok");
    });

    EXPECT_EQ(resp.status_code, 200);
}
