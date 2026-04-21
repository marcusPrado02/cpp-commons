#include <env_source.hpp>
#include <settings.hpp>

#include <gtest/gtest.h>

using namespace cpp_commons::config;
using namespace std::chrono_literals;

TEST(SettingsTest, RequireEnvThrowsWhenMissing) {
    EXPECT_THROW(require_env("CPP_COMMONS_NONEXISTENT_VAR_XYZ"), ConfigError);
}

TEST(SettingsTest, RequireEnvReturnsValue) {
    ::setenv("CPP_COMMONS_TEST_VAR", "hello", 1);
    EXPECT_EQ(require_env("CPP_COMMONS_TEST_VAR"), "hello");
    ::unsetenv("CPP_COMMONS_TEST_VAR");
}

TEST(SettingsTest, EnvAsIntParsesNumber) {
    ::setenv("CPP_COMMONS_TEST_PORT", "8080", 1);
    EXPECT_EQ(env_as<int>("CPP_COMMONS_TEST_PORT"), 8080);
    ::unsetenv("CPP_COMMONS_TEST_PORT");
}

TEST(SettingsTest, EnvAsIntThrowsOnInvalid) {
    ::setenv("CPP_COMMONS_TEST_PORT", "notanumber", 1);
    EXPECT_THROW(env_as<int>("CPP_COMMONS_TEST_PORT"), ConfigError);
    ::unsetenv("CPP_COMMONS_TEST_PORT");
}

TEST(SettingsTest, EnvOrReturnsDefault) {
    ::unsetenv("CPP_COMMONS_TEST_MISSING");
    EXPECT_EQ(env_or<int>("CPP_COMMONS_TEST_MISSING", 42), 42);
}

TEST(SettingsTest, EnvFlagTrueVariants) {
    for (const char* v : {"1", "true", "yes", "on"}) {
        ::setenv("CPP_COMMONS_TEST_FLAG", v, 1);
        EXPECT_TRUE(env_flag("CPP_COMMONS_TEST_FLAG")) << "Expected true for: " << v;
    }
    ::unsetenv("CPP_COMMONS_TEST_FLAG");
}

TEST(SettingsTest, EnvFlagFalseWhenAbsent) {
    ::unsetenv("CPP_COMMONS_TEST_FLAG");
    EXPECT_FALSE(env_flag("CPP_COMMONS_TEST_FLAG"));
}

TEST(SettingsTest, EnvDurationMsDefault) {
    ::unsetenv("CPP_COMMONS_TEST_TIMEOUT");
    EXPECT_EQ(env_duration_ms("CPP_COMMONS_TEST_TIMEOUT", 5000ms), 5000ms);
}

TEST(SettingsTest, EnvDurationMsParsed) {
    ::setenv("CPP_COMMONS_TEST_TIMEOUT", "3000", 1);
    EXPECT_EQ(env_duration_ms("CPP_COMMONS_TEST_TIMEOUT", 5000ms), 3000ms);
    ::unsetenv("CPP_COMMONS_TEST_TIMEOUT");
}

// ── parse_duration ────────────────────────────────────────────────────────────

TEST(ParseDurationTest, MillisecondSuffix) {
    EXPECT_EQ(parse_duration("x", "500ms"), 500ms);
}

TEST(ParseDurationTest, SecondSuffix) {
    EXPECT_EQ(parse_duration("x", "10s"), std::chrono::seconds{10});
}

TEST(ParseDurationTest, MinuteSuffix) {
    EXPECT_EQ(parse_duration("x", "2m"), std::chrono::minutes{2});
}

TEST(ParseDurationTest, HourSuffix) {
    EXPECT_EQ(parse_duration("x", "1h"), std::chrono::hours{1});
}

TEST(ParseDurationTest, UnknownSuffixThrows) {
    EXPECT_THROW(parse_duration("x", "10d"), ConfigError);
}

TEST(ParseDurationTest, EmptyStringThrows) {
    EXPECT_THROW(parse_duration("x", ""), ConfigError);
}

TEST(EnvAsDurationTest, ParsesFromEnv) {
    ::setenv("CPP_COMMONS_TEST_DUR", "250ms", 1);
    EXPECT_EQ(env_as<std::chrono::milliseconds>("CPP_COMMONS_TEST_DUR"), 250ms);
    ::unsetenv("CPP_COMMONS_TEST_DUR");
}

// ── ConfigValidator ───────────────────────────────────────────────────────────

TEST(ConfigValidatorTest, CheckPassesWhenNoErrors) {
    ConfigValidator v;
    ::setenv("CPP_COMMONS_V_HOST", "localhost", 1);
    v.require("CPP_COMMONS_V_HOST");
    EXPECT_NO_THROW(v.check());
    ::unsetenv("CPP_COMMONS_V_HOST");
}

TEST(ConfigValidatorTest, AccumulatesMultipleErrors) {
    ConfigValidator v;
    ::unsetenv("CPP_COMMONS_V_MISSING_A");
    ::unsetenv("CPP_COMMONS_V_MISSING_B");
    v.require("CPP_COMMONS_V_MISSING_A");
    v.require("CPP_COMMONS_V_MISSING_B");

    EXPECT_TRUE(v.has_errors());
    EXPECT_EQ(v.errors().size(), 2u);
}

TEST(ConfigValidatorTest, CheckThrowsWithAllErrors) {
    ConfigValidator v;
    ::unsetenv("CPP_COMMONS_V_MISS_X");
    ::unsetenv("CPP_COMMONS_V_MISS_Y");
    v.require("CPP_COMMONS_V_MISS_X");
    v.require("CPP_COMMONS_V_MISS_Y");

    try {
        v.check();
        FAIL() << "expected ConfigError";
    } catch (const ConfigError& e) {
        std::string msg{e.what()};
        EXPECT_NE(msg.find("CPP_COMMONS_V_MISS_X"), std::string::npos);
        EXPECT_NE(msg.find("CPP_COMMONS_V_MISS_Y"), std::string::npos);
    }
}

TEST(ConfigValidatorTest, RequireIntAggregatesOnInvalid) {
    ConfigValidator v;
    ::setenv("CPP_COMMONS_V_PORT", "bad", 1);
    v.require_int("CPP_COMMONS_V_PORT");
    EXPECT_TRUE(v.has_errors());
    EXPECT_EQ(v.errors().size(), 1u);
    ::unsetenv("CPP_COMMONS_V_PORT");
}

TEST(ConfigValidatorTest, RequireDurationAggregatesOnBadSuffix) {
    ConfigValidator v;
    ::setenv("CPP_COMMONS_V_TTL", "10d", 1);
    v.require_duration("CPP_COMMONS_V_TTL");
    EXPECT_TRUE(v.has_errors());
    ::unsetenv("CPP_COMMONS_V_TTL");
}

// ── require_validated ────────────────────────────────────────────────────────

TEST(RequireValidatedTest, PassesWhenPredicateTrue) {
    ::setenv("CPP_COMMONS_TEST_PORT", "8080", 1);
    auto v = require_validated<int>(
        "CPP_COMMONS_TEST_PORT", [](int p) { return p > 0 && p < 65536; }, "port must be 1-65535");
    EXPECT_EQ(v, 8080);
    ::unsetenv("CPP_COMMONS_TEST_PORT");
}

TEST(RequireValidatedTest, ThrowsWhenPredicateFalse) {
    ::setenv("CPP_COMMONS_TEST_PORT", "0", 1);
    EXPECT_THROW(require_validated<int>(
                     "CPP_COMMONS_TEST_PORT", [](int p) { return p > 0 && p < 65536; },
                     "port must be 1-65535"),
                 ConfigError);
    ::unsetenv("CPP_COMMONS_TEST_PORT");
}

TEST(RequireValidatedTest, ErrorMessageContainsConstraint) {
    ::setenv("CPP_COMMONS_TEST_HOST", "localhost", 1);
    try {
        require_validated<std::string>(
            "CPP_COMMONS_TEST_HOST",
            [](const std::string& s) { return s.find('.') != std::string::npos; },
            "must contain a dot");
        FAIL();
    } catch (const ConfigError& e) {
        EXPECT_NE(std::string{e.what()}.find("must contain a dot"), std::string::npos);
    }
    ::unsetenv("CPP_COMMONS_TEST_HOST");
}

TEST(RequireValidatedTest, ThrowsWhenVarMissing) {
    ::unsetenv("CPP_COMMONS_TEST_MISSING");
    EXPECT_THROW(require_validated<int>("CPP_COMMONS_TEST_MISSING", [](int) { return true; }),
                 ConfigError);
}

// ── EnvSource prefix ─────────────────────────────────────────────────────────

TEST(EnvSourcePrefixTest, NoPrefixReadsDirect) {
    ::setenv("CPP_COMMONS_EP_HOST", "direct", 1);
    EnvSource src;
    EXPECT_EQ(src.get("CPP_COMMONS_EP_HOST"), "direct");
    ::unsetenv("CPP_COMMONS_EP_HOST");
}

TEST(EnvSourcePrefixTest, PrefixPrependedToKey) {
    ::setenv("APP_PORT", "9090", 1);
    EnvSource src{"APP_"};
    EXPECT_EQ(src.prefix(), "APP_");
    EXPECT_EQ(src.get("PORT"), "9090");
    ::unsetenv("APP_PORT");
}

TEST(EnvSourcePrefixTest, UnprefixedKeyNotFound) {
    ::unsetenv("PORT");
    ::setenv("APP_PORT", "9090", 1);
    EnvSource src{"APP_"};
    EXPECT_EQ(src.get("APP_PORT"), std::nullopt);  // double-prefix not set
    ::unsetenv("APP_PORT");
}

TEST(EnvSourcePrefixTest, ReturnsNulloptForMissingKey) {
    ::unsetenv("APP_MISSING");
    EnvSource src{"APP_"};
    EXPECT_EQ(src.get("MISSING"), std::nullopt);
}
