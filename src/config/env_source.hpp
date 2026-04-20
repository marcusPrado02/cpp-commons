#pragma once
#include <optional>
#include <string>
#include <cstdlib>

namespace cpp_commons::config {

// Reads values directly from the process environment.
class EnvSource {
public:
    [[nodiscard]] std::optional<std::string> get(const std::string& key) const {
        const char* val = std::getenv(key.c_str());  // NOLINT(concurrency-mt-unsafe)
        if (!val) return std::nullopt;
        return std::string{val};
    }
};

} // namespace cpp_commons::config
