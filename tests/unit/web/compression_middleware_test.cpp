#include <compression_middleware.hpp>
#include <middleware_chain.hpp>
#include <gtest/gtest.h>

using namespace cpp_commons::web;

static HttpResponse dispatch_with_compression(const HttpRequest& req,
                                               const std::string& body = "hello world") {
    MiddlewareChain chain;
    chain.use(compression_middleware());
    Handler final_handler = [body](const HttpRequest&) {
        return HttpResponse{200, {{"Content-Type", "text/plain"}}, body};
    };
    return chain.dispatch(req, final_handler);
}

TEST(CompressionMiddlewareTest, NoAcceptEncodingPassesThrough) {
    HttpRequest req;
    auto resp = dispatch_with_compression(req);
    EXPECT_EQ(resp.status_code, 200);
    EXPECT_EQ(resp.headers.count("Content-Encoding"), 0u);
}

TEST(CompressionMiddlewareTest, NonGzipAcceptEncodingPassesThrough) {
    HttpRequest req;
    req.headers["Accept-Encoding"] = "br, deflate";
    auto resp = dispatch_with_compression(req);
    EXPECT_EQ(resp.headers.count("Content-Encoding"), 0u);
}

TEST(CompressionMiddlewareTest, EmptyBodyNotCompressed) {
    HttpRequest req;
    req.headers["Accept-Encoding"] = "gzip";
    auto resp = dispatch_with_compression(req, "");
    EXPECT_EQ(resp.headers.count("Content-Encoding"), 0u);
}

#ifdef CPP_COMMONS_HAS_ZLIB

TEST(CompressionMiddlewareTest, GzipAcceptEncodingCompressesBody) {
    HttpRequest req;
    req.headers["Accept-Encoding"] = "gzip";
    auto resp = dispatch_with_compression(req, "hello world");
    EXPECT_EQ(resp.status_code, 200);
    EXPECT_EQ(resp.headers.at("Content-Encoding"), "gzip");
    // gzip magic bytes: 0x1f 0x8b
    ASSERT_GE(resp.body.size(), 2u);
    EXPECT_EQ(static_cast<unsigned char>(resp.body[0]), 0x1fu);
    EXPECT_EQ(static_cast<unsigned char>(resp.body[1]), 0x8bu);
}

TEST(CompressionMiddlewareTest, ContentLengthUpdatedAfterCompression) {
    HttpRequest req;
    req.headers["Accept-Encoding"] = "gzip";
    auto resp = dispatch_with_compression(req, "hello world");
    auto reported = std::stoull(resp.headers.at("Content-Length"));
    EXPECT_EQ(reported, resp.body.size());
}

#endif // CPP_COMMONS_HAS_ZLIB
