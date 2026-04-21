#pragma once
#include <atomic>
#include <functional>
#include <stdexcept>
#include <string>
#include <thread>

#ifdef __linux__
#include <sys/inotify.h>
#include <unistd.h>
#endif

namespace cpp_commons::config {

// Watches a file for modifications and calls a callback when it changes.
// Uses inotify on Linux. No-op on other platforms.
//
// The callback is invoked from a background thread — callers must synchronize.
class ConfigWatcher {
public:
    using Callback = std::function<void()>;

    explicit ConfigWatcher(std::string path, Callback on_change)
        : path_(std::move(path)), callback_(std::move(on_change)) {}

    ~ConfigWatcher() { stop(); }

    ConfigWatcher(const ConfigWatcher&) = delete;
    ConfigWatcher& operator=(const ConfigWatcher&) = delete;

    void start() {
#ifdef __linux__
        if (running_.exchange(true))
            return;
        thread_ = std::thread([this] { watch_loop(); });
#endif
    }

    void stop() {
#ifdef __linux__
        if (!running_.exchange(false))
            return;
        if (inotify_fd_ >= 0) {
            ::close(inotify_fd_);
            inotify_fd_ = -1;
        }
        if (thread_.joinable())
            thread_.join();
#endif
    }

    [[nodiscard]] bool is_running() const noexcept { return running_.load(); }

private:
#ifdef __linux__
    void watch_loop() {
        inotify_fd_ = ::inotify_init1(IN_NONBLOCK);
        if (inotify_fd_ < 0) {
            running_.store(false);
            return;
        }

        int wd = ::inotify_add_watch(inotify_fd_, path_.c_str(), IN_CLOSE_WRITE | IN_MOVED_TO);
        if (wd < 0) {
            ::close(inotify_fd_);
            inotify_fd_ = -1;
            running_.store(false);
            return;
        }

        constexpr std::size_t buf_size = sizeof(inotify_event) + NAME_MAX + 1;
        char buf[buf_size];

        while (running_.load()) {
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(inotify_fd_, &rfds);
            timeval tv{0, 100'000};  // 100ms poll interval
            int sel = ::select(inotify_fd_ + 1, &rfds, nullptr, nullptr, &tv);
            if (sel <= 0)
                continue;

            ssize_t len = ::read(inotify_fd_, buf, buf_size);
            if (len <= 0)
                break;

            for (ssize_t i = 0; i < len;) {
                auto* ev = reinterpret_cast<inotify_event*>(buf + i);
                if (ev->mask & (IN_CLOSE_WRITE | IN_MOVED_TO))
                    callback_();
                i += static_cast<ssize_t>(sizeof(inotify_event) + ev->len);
            }
        }

        ::inotify_rm_watch(inotify_fd_, wd);
        ::close(inotify_fd_);
        inotify_fd_ = -1;
        running_.store(false);
    }

    std::atomic<int> inotify_fd_{-1};
    std::thread thread_;
#endif

    std::string path_;
    Callback callback_;
    std::atomic<bool> running_{false};
};

}  // namespace cpp_commons::config
