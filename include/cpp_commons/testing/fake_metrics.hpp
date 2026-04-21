/// @file fake_metrics.hpp
/// @brief NoopMetrics (discard) and SpyMetrics (capture) for MetricsPort testing.
#pragma once
#include <cpp_commons/kernel/ports/metrics_port.hpp>
#include <string>
#include <unordered_map>

namespace cpp_commons::testing {

/// @brief MetricsPort implementation that discards all observations.
class NoopMetrics {
public:
    void increment(std::string_view /*name*/)                   noexcept {}
    void gauge    (std::string_view /*name*/, double /*value*/) noexcept {}
    void histogram(std::string_view /*name*/, double /*value*/) noexcept {}
};

static_assert(kernel::MetricsPort<NoopMetrics>);

/// @brief MetricsPort implementation that records all observations for test assertions.
class SpyMetrics {
public:
    void increment(std::string_view name) {
        counters_[std::string{name}]++;
    }
    void gauge(std::string_view name, double value) {
        gauges_[std::string{name}] = value;
    }
    void histogram(std::string_view name, double value) {
        histograms_[std::string{name}].push_back(value);  // NOLINT
    }

    [[nodiscard]] int64_t counter(std::string_view name) const {
        auto it = counters_.find(std::string{name});
        return it != counters_.end() ? it->second : 0;
    }
    [[nodiscard]] double gauge_value(std::string_view name) const {
        auto it = gauges_.find(std::string{name});
        return it != gauges_.end() ? it->second : 0.0;
    }

    void clear() { counters_.clear(); gauges_.clear(); histograms_.clear(); }

private:
    std::unordered_map<std::string, int64_t>              counters_;
    std::unordered_map<std::string, double>               gauges_;
    std::unordered_map<std::string, std::vector<double>>  histograms_;
};

static_assert(kernel::MetricsPort<SpyMetrics>);

} // namespace cpp_commons::testing
