#pragma once
#include <cpp_commons/kernel/ports/logger_port.hpp>
#include "correlation_context.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace cpp_commons::observability {

// Structured JSON logger backed by spdlog.
// Satisfies kernel::LoggerPort — zero vtable overhead via concept dispatch.
class JsonLogger {
public:
    explicit JsonLogger(std::string service_name,
                        std::shared_ptr<spdlog::logger> logger = nullptr);

    using Fields = std::initializer_list<std::pair<std::string_view, std::string_view>>;

    void trace(std::string_view msg) const;
    void debug(std::string_view msg) const;
    void info(std::string_view msg)  const;
    void warn(std::string_view msg)  const;
    void error(std::string_view msg) const;

    void trace(std::string_view msg, Fields fields) const;
    void debug(std::string_view msg, Fields fields) const;
    void info(std::string_view msg,  Fields fields) const;
    void warn(std::string_view msg,  Fields fields) const;
    void error(std::string_view msg, Fields fields) const;

private:
    std::string service_name_;
    std::shared_ptr<spdlog::logger> logger_;

    void log(spdlog::level::level_enum lvl, std::string_view msg) const;
    void log(spdlog::level::level_enum lvl, std::string_view msg, Fields fields) const;
};

static_assert(kernel::LoggerPort<JsonLogger>);

} // namespace cpp_commons::observability
