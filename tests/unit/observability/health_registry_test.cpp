#include <health_registry.hpp>

#include <gtest/gtest.h>

using namespace cpp_commons::observability;

TEST(HealthRegistryTest, EmptyRegistryIsUp) {
    HealthRegistry reg;
    auto report = reg.run_all();
    EXPECT_EQ(report.overall, HealthStatus::Up);
    EXPECT_TRUE(report.checks.empty());
}

TEST(HealthRegistryTest, SingleUpCheck) {
    HealthRegistry reg;
    reg.register_check("db", [] { return HealthCheck{"db", HealthStatus::Up, "ok"}; });
    auto report = reg.run_all();
    EXPECT_EQ(report.overall, HealthStatus::Up);
    EXPECT_EQ(report.checks.size(), 1u);
}

TEST(HealthRegistryTest, DegradedCheckMakesReportDegraded) {
    HealthRegistry reg;
    reg.register_check("cache",
                       [] { return HealthCheck{"cache", HealthStatus::Degraded, "high latency"}; });
    reg.register_check("db", [] { return HealthCheck{"db", HealthStatus::Up, "ok"}; });
    auto report = reg.run_all();
    EXPECT_EQ(report.overall, HealthStatus::Degraded);
}

TEST(HealthRegistryTest, DownCheckDominatesOverDegraded) {
    HealthRegistry reg;
    reg.register_check("cache",
                       [] { return HealthCheck{"cache", HealthStatus::Degraded, "slow"}; });
    reg.register_check("db",
                       [] { return HealthCheck{"db", HealthStatus::Down, "connection refused"}; });
    auto report = reg.run_all();
    EXPECT_EQ(report.overall, HealthStatus::Down);
}
