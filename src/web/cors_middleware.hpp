#pragma once
#include "middleware_chain.hpp"
#include <string>
#include <vector>

namespace cpp_commons::web {

struct CorsOptions {
    std::vector<std::string> allowed_origins{"*"};
    std::string              allowed_methods{"GET, POST, PUT, DELETE, OPTIONS"};
    std::string              allowed_headers{"Content-Type, Authorization"};
    bool                     allow_credentials{false};
    int                      max_age_seconds{3600};
};

inline Middleware cors_middleware(CorsOptions opts = {}) {
    std::string max_age = std::to_string(opts.max_age_seconds);

    return [opts = std::move(opts), max_age = std::move(max_age)](
               const HttpRequest& req, const Handler& next) -> HttpResponse {

        auto origin = req.header("Origin").value_or("");

        std::string allow_origin;
        if (opts.allowed_origins.size() == 1 && opts.allowed_origins[0] == "*") {
            allow_origin = "*";
        } else {
            for (const auto& o : opts.allowed_origins)
                if (o == origin) { allow_origin = origin; break; }
        }

        // Preflight
        if (req.method == HttpMethod::Options) {
            HttpResponse resp{204, {}, ""};
            if (!allow_origin.empty()) {
                resp.headers["Access-Control-Allow-Origin"]  = allow_origin;
                resp.headers["Access-Control-Allow-Methods"] = opts.allowed_methods;
                resp.headers["Access-Control-Allow-Headers"] = opts.allowed_headers;
                resp.headers["Access-Control-Max-Age"]       = max_age;
                if (opts.allow_credentials)
                    resp.headers["Access-Control-Allow-Credentials"] = "true";
            }
            return resp;
        }

        auto resp = next(req);
        if (!allow_origin.empty()) {
            resp.headers["Access-Control-Allow-Origin"] = allow_origin;
            if (opts.allow_credentials)
                resp.headers["Access-Control-Allow-Credentials"] = "true";
        }
        return resp;
    };
}

} // namespace cpp_commons::web
