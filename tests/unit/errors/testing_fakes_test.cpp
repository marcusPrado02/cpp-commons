#include <chrono>

#include <cpp_commons/testing/builders.hpp>
#include <cpp_commons/testing/fake_clock.hpp>
#include <cpp_commons/testing/fake_logger.hpp>
#include <cpp_commons/testing/fake_metrics.hpp>

#include <gtest/gtest.h>

using namespace std::chrono_literals;
using namespace cpp_commons::testing;

// FakeClock tests
TEST(FakeClockTest, NowIsFixed) {
    auto t = std::chrono::system_clock::now();
    FakeClock clk{t};
    EXPECT_EQ(clk.now(), t);
}

TEST(FakeClockTest, AdvanceMovesTime) {
    auto t = std::chrono::system_clock::now();
    FakeClock clk{t};
    clk.advance(1h);
    EXPECT_EQ(clk.now(), t + 1h);
}

TEST(FakeClockTest, SetOverrides) {
    FakeClock clk;
    auto t = std::chrono::system_clock::now() + 99s;
    clk.set(t);
    EXPECT_EQ(clk.now(), t);
}

// FakeLogger tests
TEST(FakeLoggerTest, CapturesMessages) {
    FakeLogger log;
    log.info("order created");
    log.error("payment failed");
    EXPECT_EQ(log.entries().size(), 2u);
    EXPECT_EQ(log.entries()[0].level, "info");
    EXPECT_EQ(log.entries()[1].level, "error");
}

TEST(FakeLoggerTest, HasMessageFindsSubstring) {
    FakeLogger log;
    log.warn("disk usage at 95%");
    EXPECT_TRUE(log.has_message("disk"));
    EXPECT_FALSE(log.has_message("memory"));
}

TEST(FakeLoggerTest, ClearDrainsEntries) {
    FakeLogger log;
    log.debug("x");
    log.clear();
    EXPECT_TRUE(log.empty());
}

// SpyMetrics tests
TEST(SpyMetricsTest, TrackCounters) {
    SpyMetrics m;
    m.increment("requests");
    m.increment("requests");
    EXPECT_EQ(m.counter("requests"), 2);
    EXPECT_EQ(m.counter("unknown"), 0);
}

TEST(SpyMetricsTest, TrackGauge) {
    SpyMetrics m;
    m.gauge("cpu", 0.75);
    EXPECT_DOUBLE_EQ(m.gauge_value("cpu"), 0.75);
}

// Builder tests
struct Config {
    int port{8080};
    std::string host{"localhost"};
};

TEST(BuilderTest, AppliesSteps) {
    auto cfg = Builder<Config>{}
                   .with([](Config& c) { c.port = 9090; })
                   .with([](Config& c) { c.host = "example.com"; })
                   .build();
    EXPECT_EQ(cfg.port, 9090);
    EXPECT_EQ(cfg.host, "example.com");
}

TEST(BuilderTest, DefaultsWithNoSteps) {
    auto cfg = Builder<Config>{}.build();
    EXPECT_EQ(cfg.port, 8080);
}
