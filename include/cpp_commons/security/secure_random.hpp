#pragma once
#include <array>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>

namespace cpp_commons::security {

// Generates cryptographically secure random bytes from /dev/urandom.
// Throws std::runtime_error if the OS source is unavailable.
class SecureRandom {
public:
    // Returns `byte_count` cryptographically secure random bytes.
    static std::string generate_bytes(std::size_t byte_count) {
        std::ifstream urandom{"/dev/urandom", std::ios::binary};
        if (!urandom) throw std::runtime_error{"SecureRandom: cannot open /dev/urandom"};
        std::string buf(byte_count, '\0');
        if (!urandom.read(buf.data(), static_cast<std::streamsize>(byte_count)))
            throw std::runtime_error{"SecureRandom: short read from /dev/urandom"};
        return buf;
    }

    // Returns a URL-safe base64 token of `byte_count` random bytes.
    // Output length = ceil(byte_count * 4 / 3) with no padding.
    static std::string generate_token(std::size_t byte_count) {
        return base64url_encode(generate_bytes(byte_count));
    }

private:
    static std::string base64url_encode(const std::string& data) {
        static constexpr std::string_view kAlphabet =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

        std::string out;
        out.reserve((data.size() * 4 + 2) / 3);

        for (std::size_t i = 0; i < data.size(); i += 3) {
            const auto b0 = static_cast<uint8_t>(data[i]);
            const auto b1 = (i + 1 < data.size()) ? static_cast<uint8_t>(data[i + 1]) : 0u;
            const auto b2 = (i + 2 < data.size()) ? static_cast<uint8_t>(data[i + 2]) : 0u;

            out += kAlphabet[(b0 >> 2) & 0x3F];
            out += kAlphabet[((b0 & 0x03) << 4) | (b1 >> 4)];
            if (i + 1 < data.size()) out += kAlphabet[((b1 & 0x0F) << 2) | (b2 >> 6)];
            if (i + 2 < data.size()) out += kAlphabet[b2 & 0x3F];
        }
        return out;
    }
};

} // namespace cpp_commons::security
