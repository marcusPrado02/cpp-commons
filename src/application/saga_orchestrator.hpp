#pragma once
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

namespace cpp_commons::application {

struct SagaError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// Executes a sequence of steps; on failure, runs compensations in reverse order.
// Steps are plain callables; compensations (rollbacks) are registered per step.
//
// Usage:
//   SagaOrchestrator saga;
//   saga.step("debit",  [&]{ debit(acct, amt); },  [&]{ credit(acct, amt); });
//   saga.step("credit", [&]{ credit(dst, amt); },   [&]{ debit(dst, amt); });
//   saga.run(); // throws SagaError on failure, compensations already applied
class SagaOrchestrator {
public:
    using Action = std::function<void()>;

    struct Step {
        std::string name;
        Action      action;
        Action      compensation; // may be empty for non-compensatable steps
    };

    SagaOrchestrator& step(std::string name, Action action, Action compensation = {}) {
        steps_.push_back({std::move(name), std::move(action), std::move(compensation)});
        return *this;
    }

    // Execute all steps. On exception: compensate completed steps in reverse, then throw SagaError.
    void run() {
        std::size_t completed = 0;
        for (auto& s : steps_) {
            try {
                s.action();
                ++completed;
            } catch (const std::exception& e) {
                compensate(completed);
                throw SagaError{"Saga failed at step '" + s.name + "': " + e.what()};
            }
        }
    }

private:
    std::vector<Step> steps_;

    void compensate(std::size_t up_to) {
        for (std::size_t i = up_to; i > 0; --i) {
            auto& s = steps_[i - 1];
            if (s.compensation) {
                try { s.compensation(); }
                catch (...) {} // best-effort; log in production
            }
        }
    }
};

} // namespace cpp_commons::application
