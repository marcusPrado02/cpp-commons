#pragma once
#include <concepts>
#include <string_view>

namespace cpp_commons::kernel {

template<typename T>
concept LoggerPort = requires(T t, std::string_view msg) {
    { t.trace(msg) } -> std::same_as<void>;
    { t.debug(msg) } -> std::same_as<void>;
    { t.info(msg)  } -> std::same_as<void>;
    { t.warn(msg)  } -> std::same_as<void>;
    { t.error(msg) } -> std::same_as<void>;
};

} // namespace cpp_commons::kernel
