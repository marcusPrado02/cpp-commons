#include "jwt_decoder.hpp"
#include <array>

namespace cpp_commons::security {

namespace {

// Base64url decode (RFC 4648 §5) without padding.
std::string base64url_decode(std::string_view encoded) {
    std::string out;
    out.reserve(encoded.size() * 3 / 4);

    auto decode_char = [](char c) -> int {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= 'a' && c <= 'z') return c - 'a' + 26;
        if (c >= '0' && c <= '9') return c - '0' + 52;
        if (c == '+' || c == '-') return 62;
        if (c == '/' || c == '_') return 63;
        return -1;
    };

    int buf = 0;
    int bits = 0;
    for (char c : encoded) {
        int val = decode_char(c);
        if (val < 0) continue;
        buf = (buf << 6) | val;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out += static_cast<char>((buf >> bits) & 0xFF);
        }
    }
    return out;
}

} // namespace

JwtClaims decode_jwt(std::string_view token) {
    auto first_dot  = token.find('.');
    if (first_dot == std::string_view::npos)
        throw JwtError{"malformed JWT: missing first dot"};
    auto second_dot = token.find('.', first_dot + 1);
    if (second_dot == std::string_view::npos)
        throw JwtError{"malformed JWT: missing second dot"};

    auto header_b64  = token.substr(0, first_dot);
    auto payload_b64 = token.substr(first_dot + 1, second_dot - first_dot - 1);

    try {
        return {
            nlohmann::json::parse(base64url_decode(header_b64)),
            nlohmann::json::parse(base64url_decode(payload_b64)),
        };
    } catch (const nlohmann::json::exception& e) {
        throw JwtError{std::string{"malformed JWT JSON: "} + e.what()};
    }
}

} // namespace cpp_commons::security
