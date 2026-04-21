#pragma once
#include "correlation_context.hpp"

#include <initializer_list>
#include <memory>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <cpp_commons/kernel/ports/logger_port.hpp>

namespace cpp_commons::observability {

// Structured JSON logger backed by spdlog.
// Satisfies kernel::LoggerPort — zero vtable overhead via concept dispatch.
class JsonLogger {
public:
    explicit JsonLogger(std::string service_name, std::shared_ptr<spdlog::logger> logger = nullptr);

    using Fields = std::initializer_list<std::pair<std::string_view, std::string_view>>;

    void trace(std::string_view msg) const;
    void debug(std::string_view msg) const;
    void info(std::string_view msg) const;
    void warn(std::string_view msg) const;
    void error(std::string_view msg) const;

    void trace(std::string_view msg, Fields fields) const;
    void debug(std::string_view msg, Fields fields) const;
    void info(std::string_view msg, Fields fields) const;
    void warn(std::string_view msg, Fields fields) const;
    void error(std::string_view msg, Fields fields) const;

    // Set the minimum log level for this logger instance.
    // Messages below this level are silently discarded.
    void set_level(spdlog::level::level_enum lvl) { logger_->set_level(lvl); }
    [[nodiscard]] spdlog::level::level_enum level() const { return logger_->level(); }

    // Factory: stdout JSON sink + rotating file sink.
    // max_size_mb: rotate when file exceeds this size. max_files: keep this many rotated files.
    [[nodiscard]] static JsonLogger with_file(std::string service_name,
                                              const std::string& file_path,
                                              std::size_t max_size_mb = 100,
                                              std::size_t max_files = 5);

private:
    std::string service_name_;
    std::shared_ptr<spdlog::logger> logger_;

    void log(spdlog::level::level_enum lvl, std::string_view msg) const;
    void log(spdlog::level::level_enum lvl, std::string_view msg, Fields fields) const;
};

static_assert(kernel::LoggerPort<JsonLogger>);

}  // namespace cpp_commons::observability
