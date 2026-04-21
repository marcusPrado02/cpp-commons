/// @file fake_logger.hpp
/// @brief FakeLogger — in-memory LoggerPort implementation for test assertions.
#pragma once
#include <string>
#include <vector>

#include <cpp_commons/kernel/ports/logger_port.hpp>

namespace cpp_commons::testing {

struct LogEntry {
    std::string level;
    std::string message;
};

// In-memory logger that captures all messages for assertion in tests.
class FakeLogger {
public:
    void trace(std::string_view msg) { entries_.push_back({"trace", std::string{msg}}); }
    void debug(std::string_view msg) { entries_.push_back({"debug", std::string{msg}}); }
    void info(std::string_view msg) { entries_.push_back({"info", std::string{msg}}); }
    void warn(std::string_view msg) { entries_.push_back({"warn", std::string{msg}}); }
    void error(std::string_view msg) { entries_.push_back({"error", std::string{msg}}); }

    [[nodiscard]] const std::vector<LogEntry>& entries() const noexcept { return entries_; }
    [[nodiscard]] bool empty() const noexcept { return entries_.empty(); }

    [[nodiscard]] bool has_message(std::string_view needle) const {
        for (const auto& e : entries_) {
            if (e.message.find(needle) != std::string::npos)
                return true;
        }
        return false;
    }

    void clear() { entries_.clear(); }

private:
    std::vector<LogEntry> entries_;
};

static_assert(kernel::LoggerPort<FakeLogger>);

}  // namespace cpp_commons::testing
