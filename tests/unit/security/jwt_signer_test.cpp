#include <jwt_signer.hpp>
#include <jwt_decoder.hpp>
#include <gtest/gtest.h>
#include <string>

using namespace cpp_commons::security;

static const std::string kSecret = "super-secret-key-for-tests-only";

TEST(JwtSignerTest, SignedTokenCanBeDecoded) {
    JwtSigner signer{kSecret};
    nlohmann::json payload = {{"sub", "user-1"}, {"iss", "test"}, {"iat", 1000}};
    auto token = signer.sign(payload);

    // Token is three dot-separated segments.
    auto d1 = token.find('.');
    auto d2 = token.find('.', d1 + 1);
    EXPECT_NE(d1, std::string::npos);
    EXPECT_NE(d2, std::string::npos);

    auto claims = decode_jwt(token);
    EXPECT_EQ(claims.subject().value_or(""), "user-1");
    EXPECT_EQ(claims.issuer().value_or(""), "test");
}

TEST(JwtSignerTest, VerifyAcceptsOwnToken) {
    JwtSigner signer{kSecret};
    nlohmann::json payload = {{"sub", "alice"}, {"iat", 2000}};
    auto token = signer.sign(payload);

    EXPECT_NO_THROW({
        auto claims = signer.verify(token);
        EXPECT_EQ(claims.subject().value_or(""), "alice");
    });
}

TEST(JwtSignerTest, VerifyRejectsTokenFromDifferentSecret) {
    JwtSigner signer_a{kSecret};
    JwtSigner signer_b{"different-secret"};

    auto token = signer_a.sign({{"sub", "bob"}, {"iat", 3000}});
    EXPECT_THROW(signer_b.verify(token), JwtError);
}

TEST(JwtSignerTest, VerifyRejectsTamperedPayload) {
    JwtSigner signer{kSecret};
    auto token = signer.sign({{"sub", "alice"}, {"iat", 4000}});

    // Replace the payload segment with a different base64url value.
    auto d1 = token.find('.');
    auto d2 = token.find('.', d1 + 1);
    std::string tampered =
        token.substr(0, d1 + 1) +
        "eyJzdWIiOiJoYWNrZXIiLCJpYXQiOjB9" +  // {"sub":"hacker","iat":0}
        token.substr(d2);
    EXPECT_THROW(signer.verify(tampered), JwtError);
}

TEST(JwtSignerTest, SignAddsIatWhenAbsent) {
    JwtSigner signer{kSecret};
    auto token = signer.sign({{"sub", "user"}});
    auto claims = decode_jwt(token);
    EXPECT_TRUE(claims.payload.contains("iat"));
}

TEST(JwtSignerTest, SignPreservesExistingIat) {
    JwtSigner signer{kSecret};
    auto token = signer.sign({{"sub", "user"}, {"iat", 9999}});
    auto claims = decode_jwt(token);
    EXPECT_EQ(claims.payload["iat"].get<int>(), 9999);
}

TEST(JwtSignerTest, EmptySecretSignsAndVerifies) {
    JwtSigner signer{""};
    auto token = signer.sign({{"sub", "x"}, {"iat", 1}});
    EXPECT_NO_THROW(signer.verify(token));
}

TEST(JwtSignerTest, DifferentSecretsProduceDifferentTokens) {
    JwtSigner a{"secret-a"};
    JwtSigner b{"secret-b"};
    nlohmann::json payload = {{"sub", "user"}, {"iat", 100}};
    EXPECT_NE(a.sign(payload), b.sign(payload));
}
