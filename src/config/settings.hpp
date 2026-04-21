#pragma once
#include <chrono>
#include <cstdlib>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

#include <cpp_commons/errors/domain_error.hpp>

namespace cpp_commons::config {

// Thrown at startup when required env vars are missing or invalid.
class ConfigError : public std::runtime_error {
public:
    explicit ConfigError(std::string msg) : std::runtime_error{msg} {}
};

// Helper functions for typed env var access. Throw ConfigError on failure.

inline std::string require_env(const char* name) {
    const char* val = std::getenv(name);  // NOLINT(concurrency-mt-unsafe)
    if (!val || val[0] == '\0')
        throw ConfigError{std::string{"Missing required env var: "} + name};
    return val;
}

template <typename T>
T env_as(const char* name);

template <>
inline std::string env_as<std::string>(const char* name) {
    return require_env(name);
}

template <>
inline int env_as<int>(const char* name) {
    auto s = require_env(name);
    try {
        return std::stoi(s);
    } catch (...) {
        throw ConfigError{std::string{"Invalid integer for "} + name + ": " + s};
    }
}

template <>
inline uint16_t env_as<uint16_t>(const char* name) {
    int v = env_as<int>(name);
    if (v < 0 || v > 65535)
        throw ConfigError{std::string{"Port out of range for "} + name};
    return static_cast<uint16_t>(v);
}

template <>
inline bool env_as<bool>(const char* name) {
    const char* val = std::getenv(name);  // NOLINT(concurrency-mt-unsafe)
    if (!val)
        return false;
    std::string s{val};
    return s == "1" || s == "true" || s == "yes" || s == "on";
}

// Read and validate an env var with a predicate.
// Throws ConfigError with a descriptive message if the predicate returns false.
// Usage: require_validated<int>("PORT", [](int v){ return v > 0 && v < 65536; });
template <typename T>
T require_validated(const char* name, std::function<bool(const T&)> predicate,
                    const char* constraint_desc = "predicate failed") {
    auto val = env_as<T>(name);
    if (!predicate(val))
        throw ConfigError{std::string{name} + ": " + constraint_desc +
                          " (got: " + std::to_string(val) + ")"};
    return val;
}

template <>
inline std::string require_validated<std::string>(const char* name,
                                                  std::function<bool(const std::string&)> predicate,
                                                  const char* constraint_desc) {
    auto val = env_as<std::string>(name);
    if (!predicate(val))
        throw ConfigError{std::string{name} + ": " + constraint_desc + " (got: " + val + ")"};
    return val;
}

template <typename T>
T env_or(const char* name, T default_val) {
    const char* val = std::getenv(name);  // NOLINT(concurrency-mt-unsafe)
    if (!val || val[0] == '\0')
        return default_val;
    return env_as<T>(name);
}

inline bool env_flag(const char* name) {
    return env_as<bool>(name);
}

inline std::chrono::milliseconds env_duration_ms(const char* name,
                                                 std::chrono::milliseconds default_val) {
    const char* val = std::getenv(name);  // NOLINT(concurrency-mt-unsafe)
    if (!val || val[0] == '\0')
        return default_val;
    try {
        return std::chrono::milliseconds{std::stoll(val)};
    } catch (...) {
        throw ConfigError{std::string{"Invalid duration (ms) for "} + name};
    }
}

// Parse a duration string with suffix: "500ms", "10s", "2m", "1h".
// Throws ConfigError if the format is unrecognised.
inline std::chrono::milliseconds parse_duration(const char* name, std::string_view s) {
    if (s.empty())
        throw ConfigError{std::string{"Empty duration for "} + name};
    std::size_t pos = 0;
    long long count = 0;
    try {
        count = std::stoll(std::string{s}, &pos);
    } catch (...) {
        throw ConfigError{std::string{"Invalid duration for "} + name + ": " + std::string{s}};
    }
    std::string_view suffix = s.substr(pos);
    if (suffix == "ms")
        return std::chrono::milliseconds{count};
    if (suffix == "s")
        return std::chrono::seconds{count};
    if (suffix == "m")
        return std::chrono::minutes{count};
    if (suffix == "h")
        return std::chrono::hours{count};
    throw ConfigError{std::string{"Unknown duration suffix '"} + std::string{suffix} + "' for " +
                      name + " (use ms/s/m/h)"};
}

template <>
inline std::chrono::milliseconds env_as<std::chrono::milliseconds>(const char* name) {
    auto s = require_env(name);
    return parse_duration(name, s);
}

// Collects multiple config errors before throwing a combined exception.
// Usage: ConfigValidator v; v.require("HOST"); v.require_int("PORT"); v.check();
class ConfigValidator {
public:
    std::string require(const char* name) noexcept {
        const char* val = std::getenv(name);  // NOLINT(concurrency-mt-unsafe)
        if (!val || val[0] == '\0') {
            errors_.push_back(std::string{"Missing required env var: "} + name);
            return {};
        }
        return val;
    }

    int require_int(const char* name) noexcept {
        auto s = require(name);
        if (s.empty())
            return 0;
        try {
            return std::stoi(s);
        } catch (...) {
            errors_.push_back(std::string{"Invalid integer for "} + name + ": " + s);
            return 0;
        }
    }

    std::chrono::milliseconds require_duration(const char* name) noexcept {
        auto s = require(name);
        if (s.empty())
            return {};
        try {
            return parse_duration(name, s);
        } catch (const ConfigError& e) {
            errors_.push_back(e.what());
            return {};
        }
    }

    // Throws ConfigError listing all accumulated errors if any exist.
    void check() const {
        if (errors_.empty())
            return;
        std::string msg = "Config validation failed:\n";
        for (const auto& e : errors_)
            msg += "  - " + e + "\n";
        throw ConfigError{msg};
    }

    [[nodiscard]] bool has_errors() const noexcept { return !errors_.empty(); }
    [[nodiscard]] const std::vector<std::string>& errors() const noexcept { return errors_; }

private:
    std::vector<std::string> errors_;
};

}  // namespace cpp_commons::config
