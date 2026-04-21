#include <argon2_hasher.hpp>
#include <gtest/gtest.h>

using namespace cpp_commons::security;

// Use minimal params for fast tests
static Argon2Hasher fast_hasher() {
    return Argon2Hasher{1, 8192, 1};
}

TEST(Argon2HasherTest, HashProducesNonEmptyString) {
    auto h = fast_hasher();
    auto encoded = h.hash("password");
    EXPECT_FALSE(encoded.empty());
    EXPECT_NE(encoded.find("argon2id"), std::string::npos);
}

TEST(Argon2HasherTest, VerifyCorrectPasswordReturnsTrue) {
    auto h = fast_hasher();
    auto encoded = h.hash("secret");
    EXPECT_TRUE(h.verify(encoded, "secret"));
}

TEST(Argon2HasherTest, VerifyWrongPasswordReturnsFalse) {
    auto h = fast_hasher();
    auto encoded = h.hash("secret");
    EXPECT_FALSE(h.verify(encoded, "wrong"));
}

TEST(Argon2HasherTest, TwoHashesDiffer) {
    auto h = fast_hasher();
    auto e1 = h.hash("password");
    auto e2 = h.hash("password");
    EXPECT_NE(e1, e2); // different salt each time
}

TEST(Argon2HasherTest, EmptyPasswordHashAndVerify) {
    auto h = fast_hasher();
    auto encoded = h.hash("");
    EXPECT_TRUE(h.verify(encoded, ""));
    EXPECT_FALSE(h.verify(encoded, "not-empty"));
}
