#pragma once
#include <optional>
#include <string>
#include <cstdlib>

namespace cpp_commons::config {

// Reads values directly from the process environment.
// Optional prefix: EnvSource{"APP_"} maps get("PORT") → getenv("APP_PORT").
// Follows 12-factor app conventions for multi-instance environments.
class EnvSource {
public:
    EnvSource() = default;
    explicit EnvSource(std::string prefix) : prefix_{std::move(prefix)} {}

    [[nodiscard]] std::optional<std::string> get(const std::string& key) const {
        auto full_key = prefix_ + key;
        const char* val = std::getenv(full_key.c_str());  // NOLINT(concurrency-mt-unsafe)
        if (!val) return std::nullopt;
        return std::string{val};
    }

    [[nodiscard]] const std::string& prefix() const noexcept { return prefix_; }

private:
    std::string prefix_;
};

} // namespace cpp_commons::config
