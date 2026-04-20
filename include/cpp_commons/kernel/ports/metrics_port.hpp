#pragma once
#include <concepts>
#include <string_view>

namespace cpp_commons::kernel {

template<typename T>
concept MetricsPort = requires(T t, std::string_view name, double value) {
    { t.increment(name)        } -> std::same_as<void>;
    { t.gauge(name, value)     } -> std::same_as<void>;
    { t.histogram(name, value) } -> std::same_as<void>;
};

} // namespace cpp_commons::kernel
