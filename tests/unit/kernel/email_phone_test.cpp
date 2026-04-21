#include <cpp_commons/kernel/email.hpp>
#include <cpp_commons/kernel/phone_number.hpp>

#include <gtest/gtest.h>

using cpp_commons::kernel::Email;
using cpp_commons::kernel::PhoneNumber;

// ── Email ─────────────────────────────────────────────────────────────────────

TEST(EmailTest, ValidEmailParsed) {
    auto r = Email::parse("user@example.com");
    ASSERT_TRUE(r.is_ok());
    EXPECT_EQ(r.value().value(), "user@example.com");
}

TEST(EmailTest, EmptyReturnsError) {
    EXPECT_TRUE(Email::parse("").is_err());
}

TEST(EmailTest, MissingAtReturnsError) {
    EXPECT_TRUE(Email::parse("notanemail").is_err());
}

TEST(EmailTest, AtAtStartReturnsError) {
    EXPECT_TRUE(Email::parse("@example.com").is_err());
}

TEST(EmailTest, AtAtEndReturnsError) {
    EXPECT_TRUE(Email::parse("user@").is_err());
}

TEST(EmailTest, DomainWithoutDotReturnsError) {
    EXPECT_TRUE(Email::parse("user@localhost").is_err());
}

TEST(EmailTest, WhitespaceReturnsError) {
    EXPECT_TRUE(Email::parse("user @example.com").is_err());
}

TEST(EmailTest, EqualityByAddress) {
    auto a = Email::parse("a@b.com").value();
    auto b = Email::parse("a@b.com").value();
    EXPECT_EQ(a, b);
}

TEST(EmailTest, DifferentAddressesNotEqual) {
    auto a = Email::parse("a@b.com").value();
    auto b = Email::parse("c@d.com").value();
    EXPECT_NE(a, b);
}

// ── PhoneNumber ───────────────────────────────────────────────────────────────

TEST(PhoneNumberTest, ValidE164Parsed) {
    auto r = PhoneNumber::parse("+14155552671");
    ASSERT_TRUE(r.is_ok());
    EXPECT_EQ(r.value().value(), "+14155552671");
}

TEST(PhoneNumberTest, BrazilNumberParsed) {
    EXPECT_TRUE(PhoneNumber::parse("+5511999998888").is_ok());
}

TEST(PhoneNumberTest, MissingPlusReturnsError) {
    EXPECT_TRUE(PhoneNumber::parse("14155552671").is_err());
}

TEST(PhoneNumberTest, TooFewDigitsReturnsError) {
    EXPECT_TRUE(PhoneNumber::parse("+12345").is_err());
}

TEST(PhoneNumberTest, TooManyDigitsReturnsError) {
    EXPECT_TRUE(PhoneNumber::parse("+1234567890123456").is_err());
}

TEST(PhoneNumberTest, NonDigitReturnsError) {
    EXPECT_TRUE(PhoneNumber::parse("+1415555ABC1").is_err());
}

TEST(PhoneNumberTest, EqualityByNumber) {
    auto a = PhoneNumber::parse("+14155552671").value();
    auto b = PhoneNumber::parse("+14155552671").value();
    EXPECT_EQ(a, b);
}
