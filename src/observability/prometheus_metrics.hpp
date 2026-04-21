#pragma once
#include <atomic>
#include <cmath>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <cpp_commons/kernel/ports/metrics_port.hpp>

namespace cpp_commons::observability {

// In-process Prometheus text-format metrics adapter.
// Satisfies kernel::MetricsPort — compatible with concept dispatch.
//
// Thread-safe; counters/gauges use atomics, histograms use a mutex.
// Call render() to produce a Prometheus scrape endpoint response body.
class PrometheusMetrics {
public:
    // Increment a counter by 1.
    void increment(std::string_view name) {
        auto key = std::string{name};
        ensure_counter(key).fetch_add(1, std::memory_order_relaxed);
    }

    // Set a gauge to an absolute value.
    void gauge(std::string_view name, double value) {
        auto key = std::string{name};
        uint64_t bits = 0;
        static_assert(sizeof(double) == sizeof(uint64_t));
        __builtin_memcpy(&bits, &value, sizeof bits);
        ensure_gauge(key).store(bits, std::memory_order_relaxed);
    }

    // Record a histogram observation using fixed buckets.
    void histogram(std::string_view name, double value) {
        auto key = std::string{name};
        std::lock_guard lock{hist_mutex_};
        auto& h = histograms_[key];
        h.sum += value;
        h.count += 1;
        for (auto& bucket : h.buckets)
            if (value <= bucket.upper_bound)
                ++bucket.count;
    }

    // Produce the Prometheus text exposition format (for /metrics endpoint).
    [[nodiscard]] std::string render() const {
        std::ostringstream out;

        {
            std::shared_lock lock{map_mutex_};
            for (const auto& [name, counter] : counters_) {
                out << "# TYPE " << name << " counter\n";
                out << name << ' ' << counter.load(std::memory_order_relaxed) << '\n';
            }
            for (const auto& [name, bits] : gauges_) {
                double v = 0.0;
                auto raw = bits.load(std::memory_order_relaxed);
                __builtin_memcpy(&v, &raw, sizeof v);
                out << "# TYPE " << name << " gauge\n";
                out << name << ' ' << v << '\n';
            }
        }

        std::lock_guard lock{hist_mutex_};
        for (const auto& [name, h] : histograms_) {
            out << "# TYPE " << name << " histogram\n";
            for (const auto& b : h.buckets) {
                out << name << "_bucket{le=\"" << b.upper_bound << "\"} " << b.count << '\n';
            }
            out << name << "_bucket{le=\"+Inf\"} " << h.count << '\n';
            out << name << "_sum " << h.sum << '\n';
            out << name << "_count " << h.count << '\n';
        }

        return out.str();
    }

    // Reset all metrics (useful in tests).
    void reset() {
        {
            std::unique_lock lock{map_mutex_};
            for (auto& [_, c] : counters_)
                c.store(0, std::memory_order_relaxed);
            for (auto& [_, g] : gauges_)
                g.store(0, std::memory_order_relaxed);
        }
        std::lock_guard lock{hist_mutex_};
        histograms_.clear();
    }

private:
    std::atomic<uint64_t>& ensure_counter(const std::string& key) {
        {
            std::shared_lock lock{map_mutex_};
            auto it = counters_.find(key);
            if (it != counters_.end())
                return it->second;
        }
        std::unique_lock lock{map_mutex_};
        return counters_[key];  // default-init to 0
    }

    std::atomic<uint64_t>& ensure_gauge(const std::string& key) {
        {
            std::shared_lock lock{map_mutex_};
            auto it = gauges_.find(key);
            if (it != gauges_.end())
                return it->second;
        }
        std::unique_lock lock{map_mutex_};
        return gauges_[key];
    }

    struct Bucket {
        double upper_bound;
        uint64_t count{0};
    };
    struct Histogram {
        double sum{0.0};
        uint64_t count{0};
        // Default buckets in milliseconds (common for latency metrics).
        std::vector<Bucket> buckets{{1},   {5},   {10},  {25},   {50},
                                    {100}, {250}, {500}, {1000}, {5000}};
    };

    std::unordered_map<std::string, std::atomic<uint64_t>> counters_;
    std::unordered_map<std::string, std::atomic<uint64_t>> gauges_;
    std::unordered_map<std::string, Histogram> histograms_;
    mutable std::shared_mutex map_mutex_;
    mutable std::mutex hist_mutex_;
};

static_assert(kernel::MetricsPort<PrometheusMetrics>);

}  // namespace cpp_commons::observability
