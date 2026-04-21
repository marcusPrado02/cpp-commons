#pragma once
#include "http_request.hpp"
#include "http_response.hpp"
#include "middleware_chain.hpp"
#include <health_registry.hpp>
#include <memory>
#include <string>

namespace cpp_commons::web {

// Returns a Handler that calls the registry and serialises the HealthReport
// to JSON. HTTP 200 when overall == Up, 503 otherwise.
//
// Usage:
//   FakeHttpServer server;
//   server.route(HttpMethod::Get, "/health/live", health_live_handler());
//   server.route(HttpMethod::Get, "/health/ready", health_ready_handler(registry));
inline Handler health_live_handler() {
    return [](const HttpRequest&) -> HttpResponse {
        return HttpResponse{200,
                            {{"Content-Type", "application/json"}},
                            R"({"status":"up"})"};
    };
}

inline Handler health_ready_handler(
    std::shared_ptr<observability::HealthRegistry> registry)
{
    return [reg = std::move(registry)](const HttpRequest&) -> HttpResponse {
        auto report = reg->run_all();

        // Serialise checks
        std::string checks_json;
        for (const auto& c : report.checks) {
            if (!checks_json.empty()) checks_json += ',';
            checks_json += '"' + c.name + R"(":{"status":")";
            switch (c.status) {
                case observability::HealthStatus::Up:       checks_json += "up";       break;
                case observability::HealthStatus::Degraded: checks_json += "degraded"; break;
                case observability::HealthStatus::Down:     checks_json += "down";     break;
            }
            checks_json += R"(","detail":")" + c.detail + R"("})";
        }

        const bool up = report.overall == observability::HealthStatus::Up;
        const int  status_code = up ? 200 : 503;
        const std::string overall = up ? "up" : "down";

        return HttpResponse{
            status_code,
            {{"Content-Type", "application/json"}},
            R"({"status":")" + overall + R"(","checks":{)" + checks_json + "}}"
        };
    };
}

} // namespace cpp_commons::web
