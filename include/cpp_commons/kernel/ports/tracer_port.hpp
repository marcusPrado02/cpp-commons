#pragma once
#include <concepts>
#include <string_view>

namespace cpp_commons::kernel {

template<typename T>
concept TracerPort = requires(T t, std::string_view name) {
    { t.start_span(name) } -> std::same_as<void>;
    { t.end_span()       } -> std::same_as<void>;
};

} // namespace cpp_commons::kernel
