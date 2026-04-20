#pragma once
#include "http_request.hpp"
#include "http_response.hpp"
#include "middleware_chain.hpp"
#include <correlation_context.hpp>

namespace cpp_commons::web {

// Reads X-Correlation-ID / X-Tenant-ID / X-Request-ID headers and installs
// a CorrelationScope for the duration of the request.
// Falls back to generating a new context if headers are absent.
inline Middleware correlation_middleware() {
    return [](const HttpRequest& req, const Handler& next) -> HttpResponse {
        auto ctx = observability::CorrelationContext::generate();

        if (auto v = req.header("X-Correlation-ID")) ctx.correlation_id = *v;
        if (auto v = req.header("X-Tenant-ID"))      ctx.tenant_id      = *v;
        if (auto v = req.header("X-Request-ID"))     ctx.request_id     = *v;
        if (auto v = req.header("X-Trace-ID"))       ctx.trace_id       = *v;

        observability::CorrelationScope scope{ctx};
        auto resp = next(req);
        return resp;
    };
}

} // namespace cpp_commons::web
