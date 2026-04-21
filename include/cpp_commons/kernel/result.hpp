/// @file result.hpp
/// @brief Railway-oriented error handling — wraps tl::expected with a monadic API.
#pragma once
#include <tl/expected.hpp>
#include <utility>

namespace cpp_commons::kernel {

/// @brief Represents either a successful value (`T`) or an error (`E`).
///
/// Modelled on Rust's `Result<T,E>`. Use `Result::ok()` / `Result::err()` to
/// construct, `is_ok()` / `is_err()` to inspect, and `map` / `and_then` /
/// `map_err` for railway-oriented chaining.
template<typename T, typename E>
class Result {
public:
    using value_type = T;
    using error_type = E;

    static Result ok(T val)  { return Result{tl::expected<T,E>{std::move(val)}}; }
    static Result err(E e)   { return Result{tl::expected<T,E>{tl::unexpected<E>{std::move(e)}}}; }

    bool is_ok()  const noexcept { return inner_.has_value(); }
    bool is_err() const noexcept { return !inner_.has_value(); }

    T&       value() &       { return inner_.value(); }
    const T& value() const & { return inner_.value(); }
    T&&      value() &&      { return std::move(inner_).value(); }

    E&       error() &       { return inner_.error(); }
    const E& error() const & { return inner_.error(); }
    E&&      error() &&      { return std::move(inner_).error(); }

    template<typename F> auto and_then(F&& fn) &       { return inner_.and_then(std::forward<F>(fn)); }
    template<typename F> auto and_then(F&& fn) const & { return inner_.and_then(std::forward<F>(fn)); }
    template<typename F> auto or_else(F&& fn)  &       { return inner_.or_else(std::forward<F>(fn)); }
    template<typename F> auto map(F&& fn)      &       { return inner_.map(std::forward<F>(fn)); }
    template<typename F> auto map(F&& fn)      const & { return inner_.map(std::forward<F>(fn)); }

    // Transform the error value; value passes through unchanged.
    template<typename F>
    auto map_err(F&& fn) & {
        using E2 = std::invoke_result_t<F, E&>;
        if (is_err()) return Result<T, E2>::err(std::forward<F>(fn)(inner_.error()));
        return Result<T, E2>::ok(inner_.value());
    }
    template<typename F>
    auto map_err(F&& fn) const & {
        using E2 = std::invoke_result_t<F, const E&>;
        if (is_err()) return Result<T, E2>::err(std::forward<F>(fn)(inner_.error()));
        return Result<T, E2>::ok(inner_.value());
    }

    explicit operator bool() const noexcept { return is_ok(); }

private:
    explicit Result(tl::expected<T,E> inner) : inner_{std::move(inner)} {}
    tl::expected<T,E> inner_;
};

// Collapse Result<Result<T,E>,E> → Result<T,E>.
template<typename T, typename E>
Result<T, E> flatten(Result<Result<T, E>, E> r) {
    if (r.is_err()) return Result<T, E>::err(std::move(r).error());
    return std::move(r).value();
}

} // namespace cpp_commons::kernel
