#pragma once
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace cpp_commons::security {

struct JwtError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// Parsed JWT claims (header + payload only — signature not verified here).
struct JwtClaims {
    nlohmann::json header;
    nlohmann::json payload;

    [[nodiscard]] std::optional<std::string> subject() const {
        if (!payload.contains("sub")) return std::nullopt;
        return payload["sub"].get<std::string>();
    }

    [[nodiscard]] std::optional<std::string> issuer() const {
        if (!payload.contains("iss")) return std::nullopt;
        return payload["iss"].get<std::string>();
    }
};

// Decodes a JWT (header.payload.signature) without signature verification.
// Throws JwtError on malformed tokens.
// For production use behind an API gateway that already verified the signature.
[[nodiscard]] JwtClaims decode_jwt(std::string_view token);

} // namespace cpp_commons::security
