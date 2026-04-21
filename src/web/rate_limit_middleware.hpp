#pragma once
#include "middleware_chain.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include <rate_limiter.hpp>
#include <memory>
#include <string>

namespace cpp_commons::web {

// HTTP rate-limiting middleware backed by a token bucket per shared limiter.
// Returns 429 Too Many Requests when the limiter is exhausted.
// A single shared limiter applies to all requests (global rate limit).
// For per-client limiting, create a separate chain per client or wrap with a
// client-keyed map of limiters externally.
inline Middleware rate_limit_middleware(
    double max_tokens,
    double refill_rate,
    std::string retry_after_seconds = "1")
{
    auto limiter = std::make_shared<resilience::RateLimiter>(max_tokens, refill_rate);

    return [limiter, retry = std::move(retry_after_seconds)]
           (const HttpRequest& req, const Handler& next) -> HttpResponse {
        if (!limiter->try_acquire()) {
            return HttpResponse{
                429,
                {{"Retry-After", retry},
                 {"Content-Type", "application/problem+json"}},
                R"({"type":"about:blank","title":"Too Many Requests","status":429})"
            };
        }
        return next(req);
    };
}

} // namespace cpp_commons::web
