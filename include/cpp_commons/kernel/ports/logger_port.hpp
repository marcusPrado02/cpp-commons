/// @file logger_port.hpp
/// @brief LoggerPort concept — structural interface for levelled structured logging.
#pragma once
#include <concepts>
#include <string_view>

namespace cpp_commons::kernel {

/// @brief Any type with `trace/debug/info/warn/error(string_view)` satisfies this port.
template<typename T>
concept LoggerPort = requires(T t, std::string_view msg) {
    { t.trace(msg) } -> std::same_as<void>;
    { t.debug(msg) } -> std::same_as<void>;
    { t.info(msg)  } -> std::same_as<void>;
    { t.warn(msg)  } -> std::same_as<void>;
    { t.error(msg) } -> std::same_as<void>;
};

} // namespace cpp_commons::kernel
