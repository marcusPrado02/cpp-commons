#pragma once
#include "aes_gcm_provider.hpp"

#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace cpp_commons::security {

struct KeyRotationError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// Manages versioned AES-256-GCM keys for zero-downtime rotation.
// encrypt() always uses the current (latest) key and tags output with "vN:".
// decrypt() reads the version tag and selects the appropriate key.
class KeyRotationService {
public:
    // Add a key version. Versions are sorted numerically; highest = current.
    void add_key(uint32_t version, std::vector<uint8_t> key) {
        keys_.emplace(version, AesGcmProvider{std::move(key)});
    }

    // Encrypts plaintext using the current key. Output format: "vN:<base64-like blob>"
    // where N is the current version and the blob is the raw AesGcmProvider ciphertext
    // encoded as a length-prefixed byte sequence.
    [[nodiscard]] std::vector<uint8_t> encrypt(std::string_view plaintext) const {
        if (keys_.empty())
            throw KeyRotationError{"no keys registered"};

        const auto& [version, provider] = *keys_.rbegin();
        auto ciphertext = provider.encrypt(plaintext);

        // Prefix: 4-byte big-endian version number + ciphertext
        std::vector<uint8_t> output;
        output.resize(4 + ciphertext.size());
        output[0] = static_cast<uint8_t>((version >> 24) & 0xFFu);
        output[1] = static_cast<uint8_t>((version >> 16) & 0xFFu);
        output[2] = static_cast<uint8_t>((version >> 8) & 0xFFu);
        output[3] = static_cast<uint8_t>(version & 0xFFu);
        std::copy(ciphertext.begin(), ciphertext.end(), output.begin() + 4);
        return output;
    }

    // Decrypts output produced by encrypt(). Reads 4-byte version prefix.
    [[nodiscard]] std::string decrypt(const std::vector<uint8_t>& data) const {
        if (data.size() < 4)
            throw KeyRotationError{"ciphertext too short"};

        const uint32_t version =
            (static_cast<uint32_t>(data[0]) << 24) | (static_cast<uint32_t>(data[1]) << 16) |
            (static_cast<uint32_t>(data[2]) << 8) | static_cast<uint32_t>(data[3]);

        auto it = keys_.find(version);
        if (it == keys_.end())
            throw KeyRotationError{"unknown key version: " + std::to_string(version)};

        std::vector<uint8_t> ciphertext{data.begin() + 4, data.end()};
        return it->second.decrypt(ciphertext);
    }

    [[nodiscard]] uint32_t current_version() const {
        if (keys_.empty())
            throw KeyRotationError{"no keys registered"};
        return keys_.rbegin()->first;
    }

    [[nodiscard]] bool has_version(uint32_t v) const { return keys_.count(v) > 0; }

private:
    std::map<uint32_t, AesGcmProvider> keys_;
};

}  // namespace cpp_commons::security
