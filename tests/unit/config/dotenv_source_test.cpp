#include <cstdio>
#include <dotenv_source.hpp>
#include <fstream>

#include <gtest/gtest.h>

using cpp_commons::config::DotenvSource;

static const char* kTmpFile = "/tmp/cpp_commons_test.env";

static void write_env_file(const char* content) {
    std::ofstream f{kTmpFile};
    f << content;
}

TEST(DotenvSourceTest, ParsesKeyValue) {
    write_env_file("DB_URL=postgres://localhost/test\nPORT=5432\n");
    DotenvSource src{kTmpFile};
    EXPECT_EQ(src.get("DB_URL").value_or(""), "postgres://localhost/test");
    EXPECT_EQ(src.get("PORT").value_or(""), "5432");
    std::remove(kTmpFile);
}

TEST(DotenvSourceTest, SkipsComments) {
    write_env_file("# this is a comment\nKEY=value\n");
    DotenvSource src{kTmpFile};
    EXPECT_FALSE(src.get("# this is a comment").has_value());
    EXPECT_EQ(src.get("KEY").value_or(""), "value");
    std::remove(kTmpFile);
}

TEST(DotenvSourceTest, StripsQuotes) {
    write_env_file("SECRET=\"my secret value\"\n");
    DotenvSource src{kTmpFile};
    EXPECT_EQ(src.get("SECRET").value_or(""), "my secret value");
    std::remove(kTmpFile);
}

TEST(DotenvSourceTest, MissingFileIsNotAnError) {
    DotenvSource src{"/tmp/cpp_commons_definitely_missing_file.env"};
    EXPECT_FALSE(src.get("ANY").has_value());
}

TEST(DotenvSourceTest, MissingKeyReturnsNullopt) {
    write_env_file("A=1\n");
    DotenvSource src{kTmpFile};
    EXPECT_FALSE(src.get("B").has_value());
    std::remove(kTmpFile);
}
