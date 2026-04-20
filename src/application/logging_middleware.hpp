#pragma once
#include <cpp_commons/kernel/ports/logger_port.hpp>
#include "command_bus.hpp"
#include "query_bus.hpp"
#include <chrono>
#include <format>

namespace cpp_commons::application {

// Wraps a CommandBus handler with before/after log lines and elapsed time.
template <kernel::LoggerPort Logger, typename Cmd, typename Handler>
auto logging_command_handler(Logger& log, Handler handler) {
    return [&log, h = std::move(handler)](const Cmd& cmd) {
        auto t0 = std::chrono::steady_clock::now();
        log.debug(std::format("command {} started", typeid(Cmd).name()));
        h(cmd);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - t0).count();
        log.debug(std::format("command {} finished in {}ms", typeid(Cmd).name(), ms));
    };
}

// Wraps a QueryBus handler with before/after log lines and elapsed time.
template <kernel::LoggerPort Logger, typename Query, typename Result, typename Handler>
auto logging_query_handler(Logger& log, Handler handler) {
    return [&log, h = std::move(handler)](const Query& q) -> Result {
        auto t0 = std::chrono::steady_clock::now();
        log.debug(std::format("query {} started", typeid(Query).name()));
        auto result = h(q);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - t0).count();
        log.debug(std::format("query {} finished in {}ms", typeid(Query).name(), ms));
        return result;
    };
}

} // namespace cpp_commons::application
