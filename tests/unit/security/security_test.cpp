#include <api_key.hpp>
#include <jwt_decoder.hpp>
#include <pii_redactor.hpp>

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

// ── JwtDecoder adversarial ────────────────────────────────────────────────────

TEST(JwtDecoderAdversarial, EmptyTokenThrows) {
    EXPECT_THROW(decode_jwt(""), JwtError);
}

TEST(JwtDecoderAdversarial, TwoDotsEmptyPartsThrows) {
    EXPECT_THROW(decode_jwt(".."), JwtError);
}

TEST(JwtDecoderAdversarial, InvalidBase64InHeaderThrows) {
    EXPECT_THROW(decode_jwt("!!!.payload.sig"), JwtError);
}

TEST(JwtDecoderAdversarial, InvalidJsonPayloadThrows) {
    // Valid base64 but decodes to "not-json"
    // base64url of "not-json" = "bm90LWpzb24"
    EXPECT_THROW(decode_jwt("eyJhbGciOiJub25lIn0.bm90LWpzb24."), JwtError);
}

TEST(JwtDecoderAdversarial, ExtraDotsDoNotThrow) {
    // Extra segments are accepted (signature segment can be non-empty)
    EXPECT_NO_THROW(
        decode_jwt("eyJhbGciOiJub25lIiwidHlwIjoiSldUIn0"
                   ".eyJzdWIiOiJ1c2VyLTQyIiwiaXNzIjoidGVzdCJ9"
                   ".fakesig"));
}

TEST(JwtDecoderAdversarial, VeryLongTokenDoesNotCrash) {
    std::string long_junk(10000, 'A');
    EXPECT_THROW(decode_jwt(long_junk + "." + long_junk + "." + long_junk), JwtError);
}

// ── PiiRedactor adversarial ───────────────────────────────────────────────────

TEST(PiiRedactorAdversarial, EmptyStringReturnsEmpty) {
    EXPECT_EQ(PiiRedactor::redact_all(""), "");
}

TEST(PiiRedactorAdversarial, NoMatchReturnsOriginal) {
    const std::string input = "hello world, no pii here";
    EXPECT_EQ(PiiRedactor::redact_all(input), input);
}

TEST(PiiRedactorAdversarial, MultipleEmailsAllRedacted) {
    auto result = PiiRedactor::redact_email("a@b.com and c@d.org");
    EXPECT_EQ(result.find('@'), std::string::npos);
}

TEST(PiiRedactorAdversarial, MultipleCardsAllRedacted) {
    auto result = PiiRedactor::redact_card("4111 1111 1111 1111 and 5500 0000 0000 0004");
    EXPECT_EQ(result.find("1111"), std::string::npos);
    EXPECT_EQ(result.find("5500"), std::string::npos);
}

TEST(PiiRedactorAdversarial, OnlyWhitespaceReturnsWhitespace) {
    EXPECT_EQ(PiiRedactor::redact_all("   "), "   ");
}

TEST(PiiRedactorAdversarial, EmbeddedEmailInUrl) {
    auto result = PiiRedactor::redact_email("https://site.com/?user=foo@bar.com&x=1");
    EXPECT_EQ(result.find("foo@bar.com"), std::string::npos);
}
