#pragma once
#include <argon2.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace cpp_commons::security {

struct Argon2Error : std::runtime_error {
    explicit Argon2Error(int code)
        : std::runtime_error{argon2_error_message(code)} {}
};

// Argon2id password hasher.
// Default parameters are conservative (OWASP 2023 recommendations):
//   t_cost=3, m_cost=64MB, parallelism=4
class Argon2Hasher {
public:
    static constexpr uint32_t kTimeCost    = 3;
    static constexpr uint32_t kMemoryCost  = 65536; // 64 MiB
    static constexpr uint32_t kParallelism = 4;
    static constexpr uint32_t kHashLen     = 32;
    static constexpr uint32_t kSaltLen     = 16;

    explicit Argon2Hasher(uint32_t t_cost    = kTimeCost,
                          uint32_t m_cost    = kMemoryCost,
                          uint32_t threads   = kParallelism,
                          uint32_t hash_len  = kHashLen)
        : t_cost_{t_cost}, m_cost_{m_cost}
        , threads_{threads}, hash_len_{hash_len} {}

    // Produces a self-contained encoded hash string (includes salt and params).
    [[nodiscard]] std::string hash(const std::string& password) const {
        std::vector<uint8_t> salt = generate_salt();
        std::size_t enc_len = argon2_encodedlen(t_cost_, m_cost_, threads_,
                                                kSaltLen, hash_len_, Argon2_id);
        std::string encoded(enc_len, '\0');
        int rc = argon2id_hash_encoded(
            t_cost_, m_cost_, threads_,
            password.data(), password.size(),
            salt.data(), kSaltLen,
            hash_len_,
            encoded.data(), enc_len);
        if (rc != ARGON2_OK) throw Argon2Error{rc};
        // argon2 null-terminates and enc_len includes the NUL
        if (!encoded.empty() && encoded.back() == '\0')
            encoded.pop_back();
        return encoded;
    }

    // Verifies a password against an encoded hash produced by hash().
    [[nodiscard]] bool verify(const std::string& encoded,
                              const std::string& password) const {
        int rc = argon2id_verify(encoded.c_str(),
                                 password.data(), password.size());
        if (rc == ARGON2_OK)             return true;
        if (rc == ARGON2_VERIFY_MISMATCH) return false;
        throw Argon2Error{rc};
    }

private:
    uint32_t t_cost_;
    uint32_t m_cost_;
    uint32_t threads_;
    uint32_t hash_len_;

    [[nodiscard]] static std::vector<uint8_t> generate_salt() {
        std::vector<uint8_t> salt(kSaltLen);
        // Use /dev/urandom — same source as SecureRandom in this codebase.
        FILE* f = std::fopen("/dev/urandom", "rb");
        if (!f) throw std::runtime_error{"Cannot open /dev/urandom"};
        auto n = std::fread(salt.data(), 1, kSaltLen, f);
        std::fclose(f);
        if (n != kSaltLen) throw std::runtime_error{"Short read from /dev/urandom"};
        return salt;
    }
};

} // namespace cpp_commons::security
