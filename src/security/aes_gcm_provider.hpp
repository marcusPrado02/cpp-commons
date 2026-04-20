#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace cpp_commons::security {

struct EncryptionError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// AES-256-GCM authenticated encryption.
// Requires CPP_COMMONS_HAS_OPENSSL to be defined at compile time.
// Without OpenSSL, all methods throw EncryptionError.
class AesGcmProvider {
public:
    // key must be exactly 32 bytes (256 bits).
    explicit AesGcmProvider(std::vector<uint8_t> key);

    // Returns ciphertext with 12-byte IV prepended and 16-byte tag appended.
    [[nodiscard]] std::vector<uint8_t> encrypt(std::string_view plaintext) const;

    // Decrypts ciphertext produced by encrypt().
    [[nodiscard]] std::string decrypt(const std::vector<uint8_t>& ciphertext) const;

private:
    std::vector<uint8_t> key_;
};

} // namespace cpp_commons::security
