#include <set>

#include <cpp_commons/security/secure_random.hpp>

#include <gtest/gtest.h>

using cpp_commons::security::SecureRandom;

TEST(SecureRandomTest, GeneratesBytesOfRequestedLength) {
    auto bytes = SecureRandom::generate_bytes(32);
    EXPECT_EQ(bytes.size(), 32u);
}

TEST(SecureRandomTest, GeneratesUniqueBytes) {
    auto a = SecureRandom::generate_bytes(32);
    auto b = SecureRandom::generate_bytes(32);
    EXPECT_NE(a, b);
}

TEST(SecureRandomTest, TokenIsUrlSafeBase64) {
    auto token = SecureRandom::generate_token(32);
    EXPECT_FALSE(token.empty());
    for (char c : token) {
        bool safe = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
                    c == '-' || c == '_';
        EXPECT_TRUE(safe) << "Non-URL-safe character: " << c;
    }
}

TEST(SecureRandomTest, TokensAreUnique) {
    std::set<std::string> tokens;
    for (int i = 0; i < 10; ++i)
        tokens.insert(SecureRandom::generate_token(32));
    EXPECT_EQ(tokens.size(), 10u);
}

TEST(SecureRandomTest, ZeroBytesReturnsEmpty) {
    EXPECT_EQ(SecureRandom::generate_bytes(0).size(), 0u);
    EXPECT_EQ(SecureRandom::generate_token(0).size(), 0u);
}

TEST(SecureRandomTest, TokenLengthIsAtLeastByteCount) {
    // base64url of N bytes is ceil(N*4/3) chars — always >= N
    for (std::size_t n : {1u, 8u, 16u, 32u, 64u}) {
        auto token = SecureRandom::generate_token(n);
        EXPECT_GE(token.size(), n);
    }
}
