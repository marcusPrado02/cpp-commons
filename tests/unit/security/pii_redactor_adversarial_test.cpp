#include <pii_redactor.hpp>
#include <gtest/gtest.h>
#include <string>

using cpp_commons::security::PiiRedactor;

// ── Email redaction ───────────────────────────────────────────────────────────

TEST(PiiRedactorEmailTest, SingleEmail) {
    EXPECT_EQ(PiiRedactor::redact_email("contact user@example.com for info"),
              "contact [EMAIL] for info");
}

TEST(PiiRedactorEmailTest, MultipleEmailsInSameString) {
    auto result = PiiRedactor::redact_email("alice@foo.com and bob@bar.org");
    EXPECT_EQ(result.find("[EMAIL]"), 0u);
    EXPECT_NE(result.rfind("[EMAIL]"), 0u);
    EXPECT_EQ(result.find("alice"), std::string::npos);
    EXPECT_EQ(result.find("bob"), std::string::npos);
}

TEST(PiiRedactorEmailTest, NoEmailUnchanged) {
    const std::string s = "no email here";
    EXPECT_EQ(PiiRedactor::redact_email(s), s);
}

TEST(PiiRedactorEmailTest, IdempotentApplication) {
    const std::string input = "user@example.com";
    auto once  = PiiRedactor::redact_email(input);
    auto twice = PiiRedactor::redact_email(once);
    EXPECT_EQ(once, twice);
}

// ── Card redaction ────────────────────────────────────────────────────────────

TEST(PiiRedactorCardTest, SpaceSeparatedCard) {
    EXPECT_EQ(PiiRedactor::redact_card("card: 4111 1111 1111 1111 end"),
              "card: [CARD] end");
}

TEST(PiiRedactorCardTest, DashSeparatedCard) {
    EXPECT_EQ(PiiRedactor::redact_card("4111-1111-1111-1111"),
              "[CARD]");
}

TEST(PiiRedactorCardTest, ContinuousCard) {
    EXPECT_EQ(PiiRedactor::redact_card("4111111111111111"),
              "[CARD]");
}

TEST(PiiRedactorCardTest, IdempotentApplication) {
    const std::string input = "4111 1111 1111 1111";
    auto once  = PiiRedactor::redact_card(input);
    auto twice = PiiRedactor::redact_card(once);
    EXPECT_EQ(once, twice);
}

// ── redact_all combinations ───────────────────────────────────────────────────

TEST(PiiRedactorAllTest, EmailAndCardInSameString) {
    auto result = PiiRedactor::redact_all(
        "user@example.com paid 4111 1111 1111 1111");
    EXPECT_EQ(result.find("example.com"), std::string::npos);
    EXPECT_EQ(result.find("4111"), std::string::npos);
    EXPECT_NE(result.find("[EMAIL]"), std::string::npos);
    EXPECT_NE(result.find("[CARD]"), std::string::npos);
}

TEST(PiiRedactorAllTest, IdempotentApplication) {
    const std::string input = "a@b.com has card 4111 1111 1111 1111";
    auto once  = PiiRedactor::redact_all(input);
    auto twice = PiiRedactor::redact_all(once);
    EXPECT_EQ(once, twice);
}

TEST(PiiRedactorAllTest, EmptyStringUnchanged) {
    EXPECT_EQ(PiiRedactor::redact_all(""), "");
}
