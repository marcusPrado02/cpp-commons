#include "health_registry.hpp"

#include <algorithm>

namespace cpp_commons::observability {

void HealthRegistry::register_check(std::string name, CheckFn fn) {
    checks_.emplace(std::move(name), std::move(fn));
}

HealthReport HealthRegistry::run_all() const {
    HealthReport report;
    report.overall = HealthStatus::Up;

    for (const auto& [name, fn] : checks_) {
        auto check = fn();
        if (check.status == HealthStatus::Down) {
            report.overall = HealthStatus::Down;
        } else if (check.status == HealthStatus::Degraded && report.overall == HealthStatus::Up) {
            report.overall = HealthStatus::Degraded;
        }
        report.checks.push_back(std::move(check));
    }

    return report;
}

}  // namespace cpp_commons::observability
