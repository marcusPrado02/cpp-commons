#include <health_handler.hpp>
#include <fake_http_server.hpp>
#include <gtest/gtest.h>
#include <memory>

using namespace cpp_commons::web;
using namespace cpp_commons::observability;

static std::shared_ptr<HealthRegistry> make_registry(
    std::initializer_list<std::pair<std::string, HealthCheck>> checks = {})
{
    auto reg = std::make_shared<HealthRegistry>();
    for (const auto& [name, check] : checks)
        reg->register_check(name, [check] { return check; });
    return reg;
}

TEST(HealthHandlerTest, LiveAlwaysReturns200) {
    FakeHttpServer server;
    server.route(HttpMethod::Get, "/health/live", health_live_handler());

    auto resp = server.get("/health/live");
    EXPECT_EQ(resp.status_code, 200);
    EXPECT_EQ(resp.headers.at("Content-Type"), "application/json");
    EXPECT_NE(resp.body.find(R"("status":"up")"), std::string::npos);
}

TEST(HealthHandlerTest, ReadyReturns200WhenAllUp) {
    auto reg = make_registry({
        {"db",    HealthCheck{"db",    HealthStatus::Up, "ok"}},
        {"cache", HealthCheck{"cache", HealthStatus::Up, "ok"}},
    });
    FakeHttpServer server;
    server.route(HttpMethod::Get, "/health/ready", health_ready_handler(reg));

    auto resp = server.get("/health/ready");
    EXPECT_EQ(resp.status_code, 200);
    EXPECT_NE(resp.body.find(R"("status":"up")"), std::string::npos);
}

TEST(HealthHandlerTest, ReadyReturns503WhenAnyDown) {
    auto reg = make_registry({
        {"db",    HealthCheck{"db",    HealthStatus::Down, "connection refused"}},
        {"cache", HealthCheck{"cache", HealthStatus::Up,   "ok"}},
    });
    FakeHttpServer server;
    server.route(HttpMethod::Get, "/health/ready", health_ready_handler(reg));

    auto resp = server.get("/health/ready");
    EXPECT_EQ(resp.status_code, 503);
    EXPECT_NE(resp.body.find(R"("status":"down")"), std::string::npos);
}

TEST(HealthHandlerTest, ReadyBodyContainsCheckDetails) {
    auto reg = make_registry({
        {"db", HealthCheck{"db", HealthStatus::Up, "latency 2ms"}},
    });
    FakeHttpServer server;
    server.route(HttpMethod::Get, "/health/ready", health_ready_handler(reg));

    auto resp = server.get("/health/ready");
    EXPECT_NE(resp.body.find("latency 2ms"), std::string::npos);
}

TEST(HealthHandlerTest, ReadyWithNoChecksReturnsUp) {
    auto reg = make_registry();
    FakeHttpServer server;
    server.route(HttpMethod::Get, "/health/ready", health_ready_handler(reg));

    auto resp = server.get("/health/ready");
    EXPECT_EQ(resp.status_code, 200);
}
