#pragma once
#include <cpp_commons/errors/domain_error.hpp>
#include <chrono>
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace cpp_commons::config {

// Thrown at startup when required env vars are missing or invalid.
class ConfigError : public std::runtime_error {
public:
    explicit ConfigError(std::string msg) : std::runtime_error{msg} {}
};

// Helper functions for typed env var access. Throw ConfigError on failure.

inline std::string require_env(const char* name) {
    const char* val = std::getenv(name);  // NOLINT(concurrency-mt-unsafe)
    if (!val || val[0] == '\0') throw ConfigError{std::string{"Missing required env var: "} + name};
    return val;
}

template<typename T>
T env_as(const char* name);

template<>
inline std::string env_as<std::string>(const char* name) { return require_env(name); }

template<>
inline int env_as<int>(const char* name) {
    auto s = require_env(name);
    try { return std::stoi(s); }
    catch (...) { throw ConfigError{std::string{"Invalid integer for "} + name + ": " + s}; }
}

template<>
inline uint16_t env_as<uint16_t>(const char* name) {
    int v = env_as<int>(name);
    if (v < 0 || v > 65535) throw ConfigError{std::string{"Port out of range for "} + name};
    return static_cast<uint16_t>(v);
}

template<>
inline bool env_as<bool>(const char* name) {
    const char* val = std::getenv(name);  // NOLINT(concurrency-mt-unsafe)
    if (!val) return false;
    std::string s{val};
    return s == "1" || s == "true" || s == "yes" || s == "on";
}

template<typename T>
T env_or(const char* name, T default_val) {
    const char* val = std::getenv(name);  // NOLINT(concurrency-mt-unsafe)
    if (!val || val[0] == '\0') return default_val;
    return env_as<T>(name);
}

inline bool env_flag(const char* name) {
    return env_as<bool>(name);
}

inline std::chrono::milliseconds env_duration_ms(const char* name,
                                                   std::chrono::milliseconds default_val) {
    const char* val = std::getenv(name);  // NOLINT(concurrency-mt-unsafe)
    if (!val || val[0] == '\0') return default_val;
    try { return std::chrono::milliseconds{std::stoll(val)}; }
    catch (...) { throw ConfigError{std::string{"Invalid duration (ms) for "} + name}; }
}

} // namespace cpp_commons::config
