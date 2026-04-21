/// @file tracer_port.hpp
/// @brief TracerPort concept — structural interface for distributed tracing spans.
#pragma once
#include <concepts>
#include <string_view>

namespace cpp_commons::kernel {

/// @brief Any type exposing `start_span(name)` and `end_span()` satisfies this port.
template <typename T>
concept TracerPort = requires(T t, std::string_view name) {
    { t.start_span(name) } -> std::same_as<void>;
    { t.end_span() } -> std::same_as<void>;
};

}  // namespace cpp_commons::kernel
