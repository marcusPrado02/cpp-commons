#include "aes_gcm_provider.hpp"

#ifdef CPP_COMMONS_HAS_OPENSSL
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstring>
#endif

namespace cpp_commons::security {

AesGcmProvider::AesGcmProvider(std::vector<uint8_t> key) : key_(std::move(key)) {
    if (key_.size() != 32)
        throw EncryptionError{"AES-256-GCM requires a 32-byte key"};
}

#ifdef CPP_COMMONS_HAS_OPENSSL

std::vector<uint8_t> AesGcmProvider::encrypt(std::string_view plaintext) const {
    static constexpr int kIvLen  = 12;
    static constexpr int kTagLen = 16;

    std::vector<uint8_t> out(kIvLen + plaintext.size() + kTagLen);
    RAND_bytes(out.data(), kIvLen);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, kIvLen, nullptr);
    EVP_EncryptInit_ex(ctx, nullptr, nullptr, key_.data(), out.data());

    int len = 0;
    EVP_EncryptUpdate(ctx,
        out.data() + kIvLen, &len,
        reinterpret_cast<const uint8_t*>(plaintext.data()),
        static_cast<int>(plaintext.size()));

    int final_len = 0;
    EVP_EncryptFinal_ex(ctx, out.data() + kIvLen + len, &final_len);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, kTagLen, out.data() + kIvLen + len + final_len);
    EVP_CIPHER_CTX_free(ctx);
    return out;
}

std::string AesGcmProvider::decrypt(const std::vector<uint8_t>& ciphertext) const {
    static constexpr int kIvLen  = 12;
    static constexpr int kTagLen = 16;

    if (ciphertext.size() < static_cast<std::size_t>(kIvLen + kTagLen))
        throw EncryptionError{"ciphertext too short"};

    std::size_t ct_len = ciphertext.size() - kIvLen - kTagLen;
    std::string plaintext(ct_len, '\0');

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, kIvLen, nullptr);
    EVP_DecryptInit_ex(ctx, nullptr, nullptr, key_.data(), ciphertext.data());

    int len = 0;
    EVP_DecryptUpdate(ctx,
        reinterpret_cast<uint8_t*>(plaintext.data()), &len,
        ciphertext.data() + kIvLen, static_cast<int>(ct_len));

    // Set expected tag
    auto* tag_ptr = const_cast<uint8_t*>(ciphertext.data() + kIvLen + ct_len);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, kTagLen, tag_ptr);

    int final_len = 0;
    int ok = EVP_DecryptFinal_ex(ctx, reinterpret_cast<uint8_t*>(plaintext.data()) + len, &final_len);
    EVP_CIPHER_CTX_free(ctx);
    if (ok <= 0) throw EncryptionError{"authentication tag mismatch"};
    return plaintext;
}

#else

std::vector<uint8_t> AesGcmProvider::encrypt(std::string_view) const {
    throw EncryptionError{"AesGcmProvider requires OpenSSL (CPP_COMMONS_HAS_OPENSSL)"};
}

std::string AesGcmProvider::decrypt(const std::vector<uint8_t>&) const {
    throw EncryptionError{"AesGcmProvider requires OpenSSL (CPP_COMMONS_HAS_OPENSSL)"};
}

#endif

} // namespace cpp_commons::security
