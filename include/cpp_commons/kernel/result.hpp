#pragma once
#include <tl/expected.hpp>
#include <utility>

namespace cpp_commons::kernel {

template<typename T, typename E>
class Result {
public:
    static Result ok(T val)  { return Result{tl::expected<T,E>{std::move(val)}}; }
    static Result err(E err) { return Result{tl::expected<T,E>{tl::unexpected<E>{std::move(err)}}}; }

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

    explicit operator bool() const noexcept { return is_ok(); }

private:
    explicit Result(tl::expected<T,E> inner) : inner_{std::move(inner)} {}
    tl::expected<T,E> inner_;
};

} // namespace cpp_commons::kernel
