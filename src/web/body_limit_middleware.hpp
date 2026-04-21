#pragma once
#include "http_response.hpp"
#include "middleware_chain.hpp"

#include <cstddef>
#include <stdexcept>
#include <string>

namespace cpp_commons::web {

// Reject requests whose Content-Length exceeds max_bytes with 413.
// Also rejects if the actual body (after buffering) exceeds the limit.
inline Middleware body_limit_middleware(std::size_t max_bytes) {
    return [max_bytes](const HttpRequest& req, const Handler& next) -> HttpResponse {
        if (auto cl = req.header("Content-Length")) {
            try {
                auto declared = static_cast<std::size_t>(std::stoull(*cl));
                if (declared > max_bytes)
                    return HttpResponse::from_problem({"https://httpproblems.com/http-status/413",
                                                       "Request Entity Too Large", 413,
                                                       "Body exceeds maximum allowed size", ""});
            } catch (const std::invalid_argument&) {
            } catch (const std::out_of_range&) {
            }
        }

        if (req.body.size() > max_bytes)
            return HttpResponse::from_problem({"https://httpproblems.com/http-status/413",
                                               "Request Entity Too Large", 413,
                                               "Body exceeds maximum allowed size", ""});

        return next(req);
    };
}

}  // namespace cpp_commons::web
