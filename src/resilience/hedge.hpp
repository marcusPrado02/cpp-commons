#pragma once
#include <atomic>
#include <chrono>
#include <exception>
#include <future>
#include <memory>
#include <thread>

namespace cpp_commons::resilience {

// Executes fn() twice in parallel — primary immediately, hedge after hedge_delay.
// Returns the result of whichever call completes first.
// Both calls receive a shared cancelled flag they may poll to abort early.
// If both calls throw, the first exception is rethrown.
template <typename Fn>
auto hedge(std::chrono::milliseconds hedge_delay, Fn fn)
    -> std::invoke_result_t<Fn, std::atomic<bool>&>
{
    using Result = std::invoke_result_t<Fn, std::atomic<bool>&>;

    struct Shared {
        std::atomic<bool>        cancelled{false};
        std::atomic<int>         failures{0};
        std::promise<Result>     promise;
        std::exception_ptr       first_ex;
    };
    auto sh = std::make_shared<Shared>();
    auto fut = sh->promise.get_future();

    auto run = [sh](Fn fn_copy) {
        try {
            auto r = fn_copy(sh->cancelled);
            if (!sh->cancelled.exchange(true))
                sh->promise.set_value(std::move(r));
        } catch (...) {
            if (sh->failures.fetch_add(1) == 0)
                sh->first_ex = std::current_exception();
            // If both failed, second thread sets the exception.
            if (sh->failures.load() >= 2) {
                try { sh->promise.set_exception(sh->first_ex); }
                catch (const std::future_error&) {}
            }
        }
    };

    // Primary starts immediately.
    std::thread([run, fn]() mutable { run(fn); }).detach();

    // Hedge starts after delay, skipped if primary already done.
    std::thread([run, fn, hedge_delay, sh]() mutable {
        std::this_thread::sleep_for(hedge_delay);
        if (!sh->cancelled.load()) run(fn);
        // If hedge skipped after primary succeeded, failures stays < 2
        // and the promise was already fulfilled — nothing to do.
        // But if primary failed and hedge was skipped, we must set exception.
        else if (sh->failures.load() == 1) {
            try { sh->promise.set_exception(sh->first_ex); }
            catch (const std::future_error&) {}
        }
    }).detach();

    return fut.get();
}

} // namespace cpp_commons::resilience
