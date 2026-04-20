#include <api_key.hpp>
#include <pii_redactor.hpp>
#include <jwt_decoder.hpp>
#include <gtest/gtest.h>

using namespace cpp_commons::security;

// ApiKey tests
TEST(ApiKeyTest, EqualKeysCompareEqual) {
    ApiKey a{"secret123"};
    ApiKey b{"secret123"};
    EXPECT_TRUE(a == b);
}

TEST(ApiKeyTest, DifferentKeysCompareUnequal) {
    ApiKey a{"secret123"};
    ApiKey b{"secret456"};
    EXPECT_TRUE(a != b);
}

TEST(ApiKeyTest, EmptyKeyThrows) {
    EXPECT_THROW(ApiKey{""}, std::invalid_argument);
}

// PiiRedactor tests
TEST(PiiRedactorTest, RedactsEmail) {
    auto result = PiiRedactor::redact_email("Contact user@example.com for help");
    EXPECT_EQ(result, "Contact [EMAIL] for help");
}

TEST(PiiRedactorTest, RedactsCard) {
    auto result = PiiRedactor::redact_card("Card: 4111 1111 1111 1111 approved");
    EXPECT_EQ(result, "Card: [CARD] approved");
}

TEST(PiiRedactorTest, RedactAllAppliesBoth) {
    auto result = PiiRedactor::redact_all("Email user@test.com card 4111111111111111");
    EXPECT_FALSE(result.find("user@test.com") != std::string::npos);
    EXPECT_FALSE(result.find("4111111111111111") != std::string::npos);
}

// JWT tests — use a real (unsigned) JWT structure
// Header: {"alg":"none","typ":"JWT"}
// Payload: {"sub":"user-42","iss":"test"}
static const char* kTestJwt =
    "eyJhbGciOiJub25lIiwidHlwIjoiSldUIn0"
    "."
    "eyJzdWIiOiJ1c2VyLTQyIiwiaXNzIjoidGVzdCJ9"
    ".";

TEST(JwtDecoderTest, DecodesSubjectAndIssuer) {
    auto claims = decode_jwt(kTestJwt);
    EXPECT_EQ(claims.subject().value_or(""), "user-42");
    EXPECT_EQ(claims.issuer().value_or(""), "test");
}

TEST(JwtDecoderTest, ThrowsOnMalformedToken) {
    EXPECT_THROW(decode_jwt("notajwt"), JwtError);
}

TEST(JwtDecoderTest, ThrowsOnMissingSecondDot) {
    EXPECT_THROW(decode_jwt("header.payload"), JwtError);
}
