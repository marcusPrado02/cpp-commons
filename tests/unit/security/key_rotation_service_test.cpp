#include <key_rotation_service.hpp>
#include <gtest/gtest.h>

using namespace cpp_commons::security;

static std::vector<uint8_t> make_key(uint8_t fill) {
    return std::vector<uint8_t>(32, fill);
}

TEST(KeyRotationServiceTest, ThrowsWhenNoKeysRegistered) {
    KeyRotationService svc;
    EXPECT_THROW((void)svc.encrypt("data"), KeyRotationError);
    EXPECT_THROW((void)svc.current_version(), KeyRotationError);
}

TEST(KeyRotationServiceTest, CurrentVersionIsHighest) {
    KeyRotationService svc;
    svc.add_key(1, make_key(0x01));
    svc.add_key(3, make_key(0x03));
    svc.add_key(2, make_key(0x02));
    EXPECT_EQ(svc.current_version(), 3u);
}

TEST(KeyRotationServiceTest, HasVersionReturnsTrueForRegistered) {
    KeyRotationService svc;
    svc.add_key(1, make_key(0x01));
    EXPECT_TRUE(svc.has_version(1));
    EXPECT_FALSE(svc.has_version(2));
}

TEST(KeyRotationServiceTest, DecryptThrowsForUnknownVersion) {
    KeyRotationService svc;
    svc.add_key(1, make_key(0x01));
    // Craft a blob claiming version 99
    std::vector<uint8_t> fake{0, 0, 0, 99, 0, 0, 0, 0};
    EXPECT_THROW((void)svc.decrypt(fake), KeyRotationError);
}

TEST(KeyRotationServiceTest, DecryptThrowsForTooShortBlob) {
    KeyRotationService svc;
    svc.add_key(1, make_key(0x01));
    EXPECT_THROW((void)svc.decrypt({1, 2, 3}), KeyRotationError);
}

#ifdef CPP_COMMONS_HAS_OPENSSL

TEST(KeyRotationServiceTest, EncryptDecryptRoundTrip) {
    KeyRotationService svc;
    svc.add_key(1, make_key(0xAA));

    const std::string plaintext = "hello world";
    auto ciphertext = svc.encrypt(plaintext);
    auto recovered  = svc.decrypt(ciphertext);
    EXPECT_EQ(recovered, plaintext);
}

TEST(KeyRotationServiceTest, EncryptUsesCurrentKey) {
    KeyRotationService svc;
    svc.add_key(1, make_key(0x01));
    svc.add_key(2, make_key(0x02));

    auto ciphertext = svc.encrypt("data");
    // Version prefix should be 2
    uint32_t version =
        (static_cast<uint32_t>(ciphertext[0]) << 24) |
        (static_cast<uint32_t>(ciphertext[1]) << 16) |
        (static_cast<uint32_t>(ciphertext[2]) <<  8) |
         static_cast<uint32_t>(ciphertext[3]);
    EXPECT_EQ(version, 2u);
}

TEST(KeyRotationServiceTest, OldKeyStillDecryptsAfterRotation) {
    KeyRotationService svc;
    svc.add_key(1, make_key(0x01));
    auto old_ciphertext = svc.encrypt("old data");

    // Rotate: add key 2
    svc.add_key(2, make_key(0x02));

    // Old ciphertext (version 1) must still decrypt
    auto recovered = svc.decrypt(old_ciphertext);
    EXPECT_EQ(recovered, "old data");

    // New data uses key 2
    auto new_ciphertext = svc.encrypt("new data");
    EXPECT_EQ(svc.decrypt(new_ciphertext), "new data");
}

#endif // CPP_COMMONS_HAS_OPENSSL
