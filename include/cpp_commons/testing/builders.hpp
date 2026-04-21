/// @file builders.hpp
/// @brief Fluent `Builder<T>` for constructing test objects via lambda steps.
#pragma once
#include <functional>
#include <vector>

namespace cpp_commons::testing {

/// @brief Fluent builder: chain `with([](T& o){…})` calls, then `build()`.
template<typename T>
class Builder {
public:
    Builder& with(std::function<void(T&)> fn) {
        steps_.push_back(std::move(fn));
        return *this;
    }

    [[nodiscard]] T build() const {
        T obj{};
        for (const auto& step : steps_) step(obj);
        return obj;
    }

private:
    std::vector<std::function<void(T&)>> steps_;
};

} // namespace cpp_commons::testing
