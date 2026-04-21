#pragma once
#include "middleware_chain.hpp"

#ifdef CPP_COMMONS_HAS_ZLIB
#include <zlib.h>
#include <stdexcept>
#include <string>
#endif

namespace cpp_commons::web {

#ifdef CPP_COMMONS_HAS_ZLIB

namespace detail {
inline std::string gzip_compress(const std::string& input) {
    z_stream zs{};
    if (deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                     15 + 16 /*gzip*/, 8, Z_DEFAULT_STRATEGY) != Z_OK)
        throw std::runtime_error{"deflateInit2 failed"};

    zs.next_in  = reinterpret_cast<Bytef*>(const_cast<char*>(input.data()));
    zs.avail_in = static_cast<uInt>(input.size());

    std::string out;
    char buf[16384];
    int ret;
    do {
        zs.next_out  = reinterpret_cast<Bytef*>(buf);
        zs.avail_out = sizeof(buf);
        ret = deflate(&zs, Z_FINISH);
        if (out.size() < zs.total_out)
            out.append(buf, zs.total_out - out.size());
    } while (ret == Z_OK);

    deflateEnd(&zs);
    if (ret != Z_STREAM_END)
        throw std::runtime_error{"deflate did not finish"};
    return out;
}
} // namespace detail

// Compress response body with gzip when client sends Accept-Encoding: gzip.
inline Middleware compression_middleware() {
    return [](const HttpRequest& req, const Handler& next) -> HttpResponse {
        auto resp = next(req);

        auto ae = req.header("Accept-Encoding").value_or("");
        if (ae.find("gzip") == std::string::npos)
            return resp;
        if (resp.body.empty())
            return resp;

        resp.body = detail::gzip_compress(resp.body);
        resp.headers["Content-Encoding"] = "gzip";
        resp.headers["Content-Length"]   = std::to_string(resp.body.size());
        return resp;
    };
}

#else // CPP_COMMONS_HAS_ZLIB

// Pass-through when zlib is not available.
inline Middleware compression_middleware() {
    return [](const HttpRequest& req, const Handler& next) -> HttpResponse {
        return next(req);
    };
}

#endif // CPP_COMMONS_HAS_ZLIB

} // namespace cpp_commons::web
