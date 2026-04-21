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

    // map: apply fn to the value if present, returning Option<U>.
    template<typename F>
    auto map(F&& fn) & {
        using U = std::invoke_result_t<F, T&>;
        if (inner_) return Option<U>::some(std::forward<F>(fn)(*inner_));
        return Option<U>::none();
    }
    template<typename F>
    auto map(F&& fn) const & {
        using U = std::invoke_result_t<F, const T&>;
        if (inner_) return Option<U>::some(std::forward<F>(fn)(*inner_));
        return Option<U>::none();
    }

    // filter: returns none() if the predicate fails.
    template<typename Pred>
    Option filter(Pred&& pred) const {
        if (inner_.has_value() && std::forward<Pred>(pred)(*inner_))
            return *this;
        return none();
    }

    // value_or_else: lazy default — calls fn() only when none.
    template<typename F>
    T value_or_else(F&& fn) const {
        if (inner_.has_value()) return *inner_;
        return std::forward<F>(fn)();
    }

    explicit operator bool() const noexcept { return has_value(); }

private:
    explicit Option(std::optional<T> inner) : inner_{std::move(inner)} {}
    std::optional<T> inner_;
};

} // namespace cpp_commons::kernel
