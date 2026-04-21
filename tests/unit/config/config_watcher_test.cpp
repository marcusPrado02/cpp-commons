#include <config_watcher.hpp>
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <fstream>
#include <thread>
#include <unistd.h>

using namespace cpp_commons::config;
using namespace std::chrono_literals;

#ifdef __linux__

static std::string tmp_file() {
    static int n = 0;
    return "/tmp/cpp_commons_cw_" + std::to_string(::getpid()) + "_" + std::to_string(++n) + ".cfg";
}

static void write_file(const std::string& path, const std::string& content = "v1") {
    std::ofstream f(path);
    f << content;
}

TEST(ConfigWatcherTest, StartsAndStops) {
    const auto path = tmp_file();
    write_file(path);

    ConfigWatcher w{path, []{}};
    w.start();
    EXPECT_TRUE(w.is_running());
    w.stop();
    EXPECT_FALSE(w.is_running());
}

TEST(ConfigWatcherTest, CallsCallbackOnFileChange) {
    const auto path = tmp_file();
    write_file(path);

    std::atomic<int> calls{0};
    ConfigWatcher w{path, [&calls]{ calls.fetch_add(1); }};
    w.start();

    std::this_thread::sleep_for(50ms);
    write_file(path, "v2");
    std::this_thread::sleep_for(300ms);

    w.stop();
    EXPECT_GE(calls.load(), 1);
}

TEST(ConfigWatcherTest, NotCalledWhenFileNotChanged) {
    const auto path = tmp_file();
    write_file(path);

    std::atomic<int> calls{0};
    ConfigWatcher w{path, [&calls]{ calls.fetch_add(1); }};
    w.start();
    std::this_thread::sleep_for(200ms);
    w.stop();
    EXPECT_EQ(calls.load(), 0);
}

TEST(ConfigWatcherTest, DestructorStopsThread) {
    const auto path = tmp_file();
    write_file(path);
    {
        ConfigWatcher w{path, []{}};
        w.start();
        EXPECT_TRUE(w.is_running());
    } // destructor called here
    // no crash = pass
}

#else // __linux__

TEST(ConfigWatcherTest, NonLinuxIsNoOp) {
    ConfigWatcher w{"/tmp/irrelevant", []{}};
    w.start();
    EXPECT_FALSE(w.is_running()); // no-op on non-Linux
    w.stop();
}

#endif // __linux__
