#pragma once
#include <optional>
#include <utility>

namespace cpp_commons::kernel {

template<typename T>
class Option {
public:
    static Option some(T val) { return Option{std::make_optional(std::move(val))}; }
    static Option none()      { return Option{std::nullopt}; }

    bool has_value() const noexcept { return inner_.has_value(); }
    bool is_none()   const noexcept { return !inner_.has_value(); }

    T&       value() &       { return inner_.value(); }
    const T& value() const & { return inner_.value(); }

    T value_or(T default_val) const { return inner_.value_or(std::move(default_val)); }

    template<typename F> auto and_then(F&& fn) { return inner_.and_then(std::forward<F>(fn)); }
    template<typename F> auto or_else(F&& fn)  { return inner_.or_else(std::forward<F>(fn)); }
    template<typename F> auto transform(F&& fn){ return inner_.transform(std::forward<F>(fn)); }

    explicit operator bool() const noexcept { return has_value(); }

private:
    explicit Option(std::optional<T> inner) : inner_{std::move(inner)} {}
    std::optional<T> inner_;
};

} // namespace cpp_commons::kernel
