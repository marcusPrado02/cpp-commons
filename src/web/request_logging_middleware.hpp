#pragma once
#include "middleware_chain.hpp"
#include <cpp_commons/kernel/ports/logger_port.hpp>
#include <correlation_context.hpp>
#include <chrono>
#include <format>

namespace cpp_commons::web {

// Logs method + path on entry and status + elapsed on exit.
// Picks up the current correlation context automatically via JsonLogger.
template <kernel::LoggerPort Logger>
Middleware request_logging_middleware(Logger& log) {
    return [&log](const HttpRequest& req, const Handler& next) -> HttpResponse {
        const auto method_str = [&] {
            switch (req.method) {
                case HttpMethod::Get:     return "GET";
                case HttpMethod::Post:    return "POST";
                case HttpMethod::Put:     return "PUT";
                case HttpMethod::Patch:   return "PATCH";
                case HttpMethod::Delete:  return "DELETE";
                default:                 return "OTHER";
            }
        }();

        log.info(std::format("{} {} received", method_str, req.path));
        auto t0   = std::chrono::steady_clock::now();
        auto resp = next(req);
        auto ms   = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - t0).count();
        log.info(std::format("{} {} → {} ({}ms)", method_str, req.path, resp.status_code, ms));
        return resp;
    };
}

} // namespace cpp_commons::web
