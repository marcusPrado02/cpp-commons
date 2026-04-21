#pragma once
#include "jwt_decoder.hpp"
#include <nlohmann/json.hpp>
#include <chrono>
#include <string>
#include <string_view>

namespace cpp_commons::security {

// HS256 JWT signing.
// Uses a pure C++ HMAC-SHA256 implementation (no OpenSSL dependency).
// Suitable for internal service-to-service tokens; prefer asymmetric keys
// (RS256/ES256) for tokens issued to external clients.
class JwtSigner {
public:
    explicit JwtSigner(std::string secret) : secret_(std::move(secret)) {}

    // Build and sign a JWT with the given payload claims.
    // Adds "iat" (issued-at) automatically if not present.
    [[nodiscard]] std::string sign(nlohmann::json payload) const;

    // Verify the signature of a JWT and decode its claims.
    // Throws JwtError if the signature is invalid or the token is malformed.
    [[nodiscard]] JwtClaims verify(std::string_view token) const;

private:
    std::string secret_;

    static std::string base64url_encode(const std::string& data);
    static std::string hmac_sha256(std::string_view key, std::string_view msg);
};

} // namespace cpp_commons::security
