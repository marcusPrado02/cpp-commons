#include <yaml_source.hpp>
#include <gtest/gtest.h>

using cpp_commons::config::YamlSource;

TEST(YamlSourceTest, ReadsTopLevelScalar) {
    auto src = YamlSource::from_string("host: localhost\nport: 8080\n");
    EXPECT_EQ(src.get("host"), "localhost");
    EXPECT_EQ(src.get("port"), "8080");
}

TEST(YamlSourceTest, ReturnsNulloptForMissingKey) {
    auto src = YamlSource::from_string("host: localhost\n");
    EXPECT_EQ(src.get("missing"), std::nullopt);
}

TEST(YamlSourceTest, DotNotationAccessesNestedKey) {
    const std::string yaml = R"(
database:
  host: db.internal
  port: 5432
)";
    auto src = YamlSource::from_string(yaml);
    EXPECT_EQ(src.get("database.host"), "db.internal");
    EXPECT_EQ(src.get("database.port"), "5432");
}

TEST(YamlSourceTest, ReturnsNulloptForMissingNestedKey) {
    const std::string yaml = "database:\n  host: localhost\n";
    auto src = YamlSource::from_string(yaml);
    EXPECT_EQ(src.get("database.missing"), std::nullopt);
    EXPECT_EQ(src.get("missing.key"), std::nullopt);
}

TEST(YamlSourceTest, ReturnsNulloptForNonScalarValue) {
    // A list is not a scalar — should return nullopt
    const std::string yaml = "items:\n  - a\n  - b\n";
    auto src = YamlSource::from_string(yaml);
    EXPECT_EQ(src.get("items"), std::nullopt);
}

TEST(YamlSourceTest, IntegerValueReturnedAsString) {
    auto src = YamlSource::from_string("count: 42\n");
    EXPECT_EQ(src.get("count"), "42");
}

TEST(YamlSourceTest, ThrowsOnInvalidYaml) {
    EXPECT_THROW(YamlSource::from_string("key: [unclosed"), std::runtime_error);
}

TEST(YamlSourceTest, ThrowsOnMissingFile) {
    EXPECT_THROW(YamlSource{"/nonexistent/path/config.yaml"}, std::runtime_error);
}
