#pragma once
#include "correlation_context.hpp"

#include <asio/execution_context.hpp>

namespace cpp_commons::observability {

// Wraps any Asio executor and captures a CorrelationContext snapshot at
// construction. Before every dispatched function, restores that snapshot via
// CorrelationScope so correlation IDs survive coroutine suspension points and
// thread-pool migration — unlike a raw thread_local approach.
template <typename InnerExecutor>
class ContextAwareExecutor {
public:
    using inner_executor_type = InnerExecutor;

    ContextAwareExecutor(InnerExecutor inner, const CorrelationContext& ctx)
        : inner_(std::move(inner)), ctx_(ctx) {}

    [[nodiscard]] static ContextAwareExecutor from_current(InnerExecutor inner) {
        return {std::move(inner), current_correlation()};
    }

    [[nodiscard]] asio::execution_context& context() const noexcept { return inner_.context(); }
    void on_work_started() const noexcept { inner_.on_work_started(); }
    void on_work_finished() const noexcept { inner_.on_work_finished(); }

    template <typename Func, typename Alloc>
    void dispatch(Func&& f, const Alloc& a) const {
        inner_.dispatch(wrap(std::forward<Func>(f)), a);
    }

    template <typename Func, typename Alloc>
    void post(Func&& f, const Alloc& a) const {
        inner_.post(wrap(std::forward<Func>(f)), a);
    }

    template <typename Func, typename Alloc>
    void defer(Func&& f, const Alloc& a) const {
        inner_.defer(wrap(std::forward<Func>(f)), a);
    }

    [[nodiscard]] const CorrelationContext& context_snapshot() const noexcept { return ctx_; }
    [[nodiscard]] const InnerExecutor& inner() const noexcept { return inner_; }

    bool operator==(const ContextAwareExecutor& rhs) const noexcept {
        return inner_ == rhs.inner_ && ctx_.correlation_id == rhs.ctx_.correlation_id;
    }
    bool operator!=(const ContextAwareExecutor& rhs) const noexcept { return !(*this == rhs); }

private:
    template <typename Func>
    auto wrap(Func&& f) const {
        CorrelationContext snap = ctx_;
        return [snap, fn = std::forward<Func>(f)]() mutable {
            CorrelationScope scope{snap};
            std::move(fn)();
        };
    }

    InnerExecutor inner_;
    CorrelationContext ctx_;
};

// Deduction helper: builds a ContextAwareExecutor from any executor,
// capturing the currently active correlation context.
template <typename Executor>
[[nodiscard]] auto make_context_executor(Executor ex) {
    return ContextAwareExecutor<Executor>::from_current(std::move(ex));
}

}  // namespace cpp_commons::observability
