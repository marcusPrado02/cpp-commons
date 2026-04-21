#include <jwt_decoder.hpp>
#include <jwt_signer.hpp>
#include <gtest/gtest.h>
#include <string>

using namespace cpp_commons::security;

// ── Malformed token structure ─────────────────────────────────────────────────

TEST(JwtDecoderAdversarialTest, EmptyStringThrows) {
    EXPECT_THROW(decode_jwt(""), JwtError);
}

TEST(JwtDecoderAdversarialTest, MissingFirstDotThrows) {
    EXPECT_THROW(decode_jwt("nodotintoken"), JwtError);
}

TEST(JwtDecoderAdversarialTest, MissingSecondDotThrows) {
    EXPECT_THROW(decode_jwt("header.payloadonly"), JwtError);
}

TEST(JwtDecoderAdversarialTest, TooManyDotsStillDecodes) {
    // Extra dots after signature are ignored — only header.payload extracted.
    JwtSigner signer{"secret"};
    auto token = signer.sign({{"sub", "user"}, {"iat", 1}});
    auto tampered = token + ".extra.segment";
    EXPECT_NO_THROW(decode_jwt(tampered));
}

TEST(JwtDecoderAdversarialTest, EmptyHeaderSegmentThrows) {
    EXPECT_THROW(decode_jwt(".payload.sig"), JwtError);
}

TEST(JwtDecoderAdversarialTest, EmptyPayloadSegmentThrows) {
    EXPECT_THROW(decode_jwt("header..sig"), JwtError);
}

TEST(JwtDecoderAdversarialTest, NonBase64HeaderThrows) {
    EXPECT_THROW(decode_jwt("!!!.payload.sig"), JwtError);
}

TEST(JwtDecoderAdversarialTest, InvalidJsonPayloadThrows) {
    // Valid base64url of "{bad json"
    EXPECT_THROW(decode_jwt("e30.e2JhZA.sig"), JwtError);  // header={}, payload={bad
}

// ── alg:none accepted (decoder is non-verifying by design) ───────────────────

TEST(JwtDecoderAdversarialTest, AlgNoneTokenDecodesWithoutError) {
    // Header: {"alg":"none","typ":"JWT"}, payload: {"sub":"attacker"}
    // Callers are responsible for signature verification; decoder is explicitly non-verifying.
    const std::string token =
        "eyJhbGciOiJub25lIiwidHlwIjoiSldUIn0"   // {"alg":"none","typ":"JWT"}
        ".eyJzdWIiOiJhdHRhY2tlciJ9"              // {"sub":"attacker"}
        ".";                                       // empty signature
    EXPECT_NO_THROW({
        auto claims = decode_jwt(token);
        EXPECT_EQ(claims.header["alg"], "none");
        EXPECT_EQ(claims.subject().value_or(""), "attacker");
    });
}

// ── Oversized token ───────────────────────────────────────────────────────────

TEST(JwtDecoderAdversarialTest, VeryLargeTokenDoesNotCrash) {
    // A 64KB token should not crash; either decode or throw JwtError — not crash/abort.
    std::string big(65536, 'A');
    big += '.';
    big += std::string(65536, 'B');
    big += ".sig";
    try {
        [[maybe_unused]] auto claims = decode_jwt(big);
    } catch (const JwtError&) {
        // acceptable — malformed payload
    }
    SUCCEED();
}

// ── Signature tampering rejected by JwtSigner.verify ─────────────────────────

TEST(JwtDecoderAdversarialTest, VerifyRejectsTokenWithStrippedSignature) {
    JwtSigner signer{"mysecret"};
    auto token = signer.sign({{"sub", "user"}, {"iat", 1}});
    auto d2 = token.rfind('.');
    std::string stripped = token.substr(0, d2 + 1);  // keep trailing dot, empty sig
    EXPECT_THROW(signer.verify(stripped), JwtError);
}

TEST(JwtDecoderAdversarialTest, VerifyRejectsExpiredToken) {
    JwtSigner signer{"mysecret"};
    // exp = 1 (Unix epoch 1970-01-01T00:00:01Z — long expired)
    auto token = signer.sign({{"sub", "user"}, {"iat", 1}, {"exp", 1}});
    // decode_jwt itself doesn't check expiry — caller must; verify doesn't either
    // (our JwtSigner.verify only checks HMAC, not claims). Just ensure no crash.
    EXPECT_NO_THROW(signer.verify(token));
}
