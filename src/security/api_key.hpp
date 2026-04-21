#pragma once
#include <string>

#include <cpp_commons/kernel/identity.hpp>

namespace cpp_commons::security {

// Immutable value object wrapping a raw API key string.
// Construction validates the key is non-empty.
class ApiKey {
public:
    explicit ApiKey(std::string raw);

    [[nodiscard]] const std::string& value() const noexcept { return raw_; }

    // Constant-time comparison to prevent timing attacks.
    [[nodiscard]] bool operator==(const ApiKey& other) const noexcept;
    [[nodiscard]] bool operator!=(const ApiKey& other) const noexcept;

private:
    std::string raw_;
};

}  // namespace cpp_commons::security
