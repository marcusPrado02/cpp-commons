#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace cpp_commons::observability {

enum class HealthStatus { Up, Degraded, Down };

struct HealthCheck {
    std::string name;
    HealthStatus status;
    std::string detail;
};

struct HealthReport {
    HealthStatus overall;
    std::vector<HealthCheck> checks;
};

// Registry of named health-check functions.
// Callers register lambdas; run_all() invokes each and aggregates status.
class HealthRegistry {
public:
    using CheckFn = std::function<HealthCheck()>;

    void register_check(std::string name, CheckFn fn);
    [[nodiscard]] HealthReport run_all() const;

private:
    std::unordered_map<std::string, CheckFn> checks_;
};

} // namespace cpp_commons::observability
