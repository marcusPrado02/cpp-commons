#include <json_logger.hpp>
#include <gtest/gtest.h>
#include <spdlog/sinks/ostream_sink.h>
#include <fstream>
#include <sstream>
#include <filesystem>

using namespace cpp_commons::observability;
namespace fs = std::filesystem;

// Helper: build a JsonLogger that writes to an in-memory stream
static JsonLogger make_stream_logger(const std::string& name, std::ostringstream& oss) {
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    auto raw   = std::make_shared<spdlog::logger>(name, sink);
    raw->set_pattern("%v");
    raw->set_level(spdlog::level::trace);
    return JsonLogger{name, raw};
}

// ── set_level / level ─────────────────────────────────────────────────────────

TEST(JsonLoggerTest, DefaultLevelIsTrace) {
    std::ostringstream oss;
    auto logger = make_stream_logger("svc-level-default", oss);
    EXPECT_EQ(logger.level(), spdlog::level::trace);
}

TEST(JsonLoggerTest, SetLevelFiltersLowerMessages) {
    std::ostringstream oss;
    auto logger = make_stream_logger("svc-level-filter", oss);

    logger.set_level(spdlog::level::warn);
    EXPECT_EQ(logger.level(), spdlog::level::warn);

    logger.debug("should be dropped");
    logger.info("also dropped");
    EXPECT_TRUE(oss.str().empty());

    logger.warn("visible");
    EXPECT_FALSE(oss.str().empty());
}

TEST(JsonLoggerTest, SetLevelAllowsMessagesAtOrAboveLevel) {
    std::ostringstream oss;
    auto logger = make_stream_logger("svc-level-pass", oss);

    logger.set_level(spdlog::level::info);
    logger.info("at level");
    logger.warn("above level");
    logger.error("error level");

    const auto out = oss.str();
    EXPECT_NE(out.find("at level"), std::string::npos);
    EXPECT_NE(out.find("above level"), std::string::npos);
    EXPECT_NE(out.find("error level"), std::string::npos);
}

// ── with_file() ───────────────────────────────────────────────────────────────

TEST(JsonLoggerTest, WithFileCreatesLogFile) {
    const std::string path = "/tmp/cpp_commons_test_with_file.log";
    fs::remove(path);

    {
        auto logger = JsonLogger::with_file("svc-file", path, 1, 3);
        logger.info("hello from file");
    }
    spdlog::drop("svc-file");

    EXPECT_TRUE(fs::exists(path));
    fs::remove(path);
}

TEST(JsonLoggerTest, WithFileWritesJsonToFile) {
    const std::string path = "/tmp/cpp_commons_test_json_content.log";
    fs::remove(path);

    {
        auto logger = JsonLogger::with_file("svc-json-content", path, 1, 3);
        logger.info("structured message");
    }
    spdlog::drop("svc-json-content");

    std::ifstream f(path);
    ASSERT_TRUE(f.is_open());
    std::string line;
    std::getline(f, line);
    EXPECT_NE(line.find("structured message"), std::string::npos);
    fs::remove(path);
}

TEST(JsonLoggerTest, WithFileLevelCanBeChanged) {
    const std::string path = "/tmp/cpp_commons_test_level_file.log";
    fs::remove(path);

    {
        auto logger = JsonLogger::with_file("svc-level-file", path, 1, 3);
        logger.set_level(spdlog::level::err);
        logger.debug("filtered");
        logger.info("filtered");
        logger.error("kept");
    }
    spdlog::drop("svc-level-file");

    std::ifstream f(path);
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    EXPECT_EQ(content.find("filtered"), std::string::npos);
    EXPECT_NE(content.find("kept"), std::string::npos);
    fs::remove(path);
}
