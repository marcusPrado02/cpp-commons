#include <atomic>
#include <prometheus_metrics.hpp>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

using cpp_commons::observability::PrometheusMetrics;

TEST(PrometheusMetricsTest, CounterAppearsInOutput) {
    PrometheusMetrics m;
    m.increment("http_requests_total");
    m.increment("http_requests_total");

    auto out = m.render();
    EXPECT_NE(out.find("http_requests_total"), std::string::npos);
    EXPECT_NE(out.find("# TYPE http_requests_total counter"), std::string::npos);
    EXPECT_NE(out.find("http_requests_total 2"), std::string::npos);
}

TEST(PrometheusMetricsTest, GaugeAppearsInOutput) {
    PrometheusMetrics m;
    m.gauge("active_connections", 42.5);

    auto out = m.render();
    EXPECT_NE(out.find("# TYPE active_connections gauge"), std::string::npos);
    EXPECT_NE(out.find("active_connections"), std::string::npos);
}

TEST(PrometheusMetricsTest, HistogramAppearsInOutput) {
    PrometheusMetrics m;
    m.histogram("request_duration_ms", 15.0);
    m.histogram("request_duration_ms", 150.0);

    auto out = m.render();
    EXPECT_NE(out.find("# TYPE request_duration_ms histogram"), std::string::npos);
    EXPECT_NE(out.find("request_duration_ms_sum"), std::string::npos);
    EXPECT_NE(out.find("request_duration_ms_count 2"), std::string::npos);
    EXPECT_NE(out.find("+Inf"), std::string::npos);
}

TEST(PrometheusMetricsTest, SatisfiesMetricsPortConcept) {
    // Verified at compile time via static_assert in the header.
    // This test documents that expectation at runtime too.
    SUCCEED();
}

TEST(PrometheusMetricsTest, ResetClearsCounters) {
    PrometheusMetrics m;
    m.increment("x");
    m.increment("x");
    m.reset();

    auto out = m.render();
    EXPECT_NE(out.find("x 0"), std::string::npos);
}

TEST(PrometheusMetricsTest, MultipleMetricsCoexist) {
    PrometheusMetrics m;
    m.increment("req");
    m.gauge("mem", 1024.0);
    m.histogram("lat", 5.0);

    auto out = m.render();
    EXPECT_NE(out.find("req"), std::string::npos);
    EXPECT_NE(out.find("mem"), std::string::npos);
    EXPECT_NE(out.find("lat"), std::string::npos);
}

TEST(PrometheusMetricsTest, ThreadSafeCounterUnderContention) {
    PrometheusMetrics m;
    std::atomic<int> done{0};
    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back([&] {
            for (int j = 0; j < 100; ++j)
                m.increment("concurrent");
        });
    }
    for (auto& t : threads)
        t.join();

    auto out = m.render();
    EXPECT_NE(out.find("concurrent 800"), std::string::npos);
}
