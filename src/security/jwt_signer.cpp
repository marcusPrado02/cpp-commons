#include "jwt_signer.hpp"

#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace cpp_commons::security {

// ── SHA-256 (RFC 6234) ────────────────────────────────────────────────────────

namespace {

constexpr std::array<uint32_t, 64> k256 = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

constexpr uint32_t rotr(uint32_t x, unsigned n) {
    return (x >> n) | (x << (32 - n));
}

std::string sha256(std::string_view msg) {
    uint32_t h0 = 0x6a09e667, h1 = 0xbb67ae85, h2 = 0x3c6ef372, h3 = 0xa54ff53a;
    uint32_t h4 = 0x510e527f, h5 = 0x9b05688c, h6 = 0x1f83d9ab, h7 = 0x5be0cd19;

    // Pre-processing: padding
    auto len = msg.size();
    std::string padded(msg);
    padded += '\x80';
    while ((padded.size() % 64) != 56)
        padded += '\x00';
    uint64_t bit_len = static_cast<uint64_t>(len) * 8;
    for (unsigned i = 0; i < 8; ++i)
        padded += static_cast<char>((bit_len >> ((7u - i) * 8u)) & 0xFFu);

    for (std::size_t chunk = 0; chunk < padded.size(); chunk += 64) {
        std::array<uint32_t, 64> w{};
        for (std::size_t i = 0; i < 16; ++i) {
            std::size_t base = chunk + i * 4;
            w[i] = (static_cast<uint32_t>(static_cast<uint8_t>(padded[base])) << 24u) |
                   (static_cast<uint32_t>(static_cast<uint8_t>(padded[base + 1])) << 16u) |
                   (static_cast<uint32_t>(static_cast<uint8_t>(padded[base + 2])) << 8u) |
                   static_cast<uint32_t>(static_cast<uint8_t>(padded[base + 3]));
        }
        for (std::size_t i = 16; i < 64; ++i) {
            uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3u);
            uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10u);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        uint32_t a = h0, b = h1, c = h2, d = h3, e = h4, f = h5, g = h6, h = h7;
        for (std::size_t i = 0; i < 64; ++i) {
            uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            uint32_t ch = (e & f) ^ (~e & g);
            uint32_t t1 = h + S1 + ch + k256[i] + w[i];
            uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t t2 = S0 + maj;
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
        h5 += f;
        h6 += g;
        h7 += h;
    }

    std::string digest(32, '\0');
    auto put = [&](std::size_t off, uint32_t v) {
        digest[off] = static_cast<char>((v >> 24u) & 0xFFu);
        digest[off + 1] = static_cast<char>((v >> 16u) & 0xFFu);
        digest[off + 2] = static_cast<char>((v >> 8u) & 0xFFu);
        digest[off + 3] = static_cast<char>(v & 0xFFu);
    };
    put(0, h0);
    put(4, h1);
    put(8, h2);
    put(12, h3);
    put(16, h4);
    put(20, h5);
    put(24, h6);
    put(28, h7);
    return digest;
}

std::string hmac_sha256_impl(std::string_view key, std::string_view msg) {
    constexpr std::size_t block = 64;
    std::string k(key);
    if (k.size() > block)
        k = sha256(k);
    k.resize(block, '\0');

    std::string ipad(block, '\x36'), opad(block, '\x5c');
    for (std::size_t i = 0; i < block; ++i) {
        ipad[i] = static_cast<char>(static_cast<unsigned char>(ipad[i]) ^
                                    static_cast<unsigned char>(k[i]));
        opad[i] = static_cast<char>(static_cast<unsigned char>(opad[i]) ^
                                    static_cast<unsigned char>(k[i]));
    }
    return sha256(opad + sha256(ipad + std::string(msg)));
}

std::string base64url_encode_impl(const std::string& data) {
    static const char* alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::string out;
    out.reserve((data.size() * 4 + 2) / 3);
    int val = 0, bits = -6;
    for (char raw : data) {
        val = (val << 8) + static_cast<int>(static_cast<unsigned char>(raw));
        bits += 8;
        while (bits >= 0) {
            out += alpha[(val >> bits) & 0x3F];
            bits -= 6;
        }
    }
    if (bits > -6)
        out += alpha[((val << 8) >> (bits + 8)) & 0x3F];
    // no padding ('=' stripped for JWT)
    return out;
}

// Constant-time comparison to resist timing attacks on HMAC verification.
bool constant_time_equal(const std::string& a, const std::string& b) {
    if (a.size() != b.size())
        return false;
    unsigned char diff = 0;
    for (std::size_t i = 0; i < a.size(); ++i)
        diff = static_cast<unsigned char>(
            diff | (static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i])));
    return diff == 0;
}

}  // namespace

// ── JwtSigner ─────────────────────────────────────────────────────────────────

std::string JwtSigner::hmac_sha256(std::string_view key, std::string_view msg) {
    return hmac_sha256_impl(key, msg);
}

std::string JwtSigner::base64url_encode(const std::string& data) {
    return base64url_encode_impl(data);
}

std::string JwtSigner::sign(nlohmann::json payload) const {
    if (!payload.contains("iat")) {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        payload["iat"] = std::chrono::duration_cast<std::chrono::seconds>(now).count();
    }

    nlohmann::json header = {{"alg", "HS256"}, {"typ", "JWT"}};
    std::string h_b64 = base64url_encode(header.dump());
    std::string p_b64 = base64url_encode(payload.dump());
    std::string signing_input = h_b64 + '.' + p_b64;
    std::string sig = base64url_encode(hmac_sha256(secret_, signing_input));
    return signing_input + '.' + sig;
}

JwtClaims JwtSigner::verify(std::string_view token) const {
    auto first_dot = token.find('.');
    if (first_dot == std::string_view::npos)
        throw JwtError{"malformed JWT: missing first dot"};
    auto second_dot = token.find('.', first_dot + 1);
    if (second_dot == std::string_view::npos)
        throw JwtError{"malformed JWT: missing second dot"};

    auto signing_input = token.substr(0, second_dot);
    auto sig_b64 = std::string{token.substr(second_dot + 1)};
    auto expected_sig = base64url_encode(hmac_sha256(secret_, signing_input));

    if (!constant_time_equal(sig_b64, expected_sig))
        throw JwtError{"JWT signature verification failed"};

    return decode_jwt(token);
}

}  // namespace cpp_commons::security
