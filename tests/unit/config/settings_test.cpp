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
