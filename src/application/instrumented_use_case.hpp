#pragma once
#include "use_case.hpp"
#include <cpp_commons/kernel/ports/metrics_port.hpp>
#include <chrono>
#include <string>

namespace cpp_commons::application {

// Wraps any UseCase and automatically records:
//   <name>.calls      — incremented on every execute()
//   <name>.errors     — incremented when execute() returns an error
//   <name>.duration_ms — histogram of execution time in milliseconds
//
// Usage:
//   PlaceOrderUseCase inner{...};
//   InstrumentedUseCase decorated{inner, metrics, "place_order"};
//   auto result = decorated.execute(input);
template <typename In, typename Out, typename Err, kernel::MetricsPort Metrics>
class InstrumentedUseCase : public UseCase<In, Out, Err> {
public:
    using Result = typename UseCase<In, Out, Err>::Result;

    InstrumentedUseCase(UseCase<In, Out, Err>& inner,
                        Metrics& metrics,
                        std::string name)
        : inner_{inner}
        , metrics_{metrics}
        , calls_{name + ".calls"}
        , errors_{name + ".errors"}
        , duration_{name + ".duration_ms"}
    {}

    [[nodiscard]] Result execute(const In& input) override {
        metrics_.increment(calls_);

        const auto start = std::chrono::steady_clock::now();
        auto result      = inner_.execute(input);
        const auto end   = std::chrono::steady_clock::now();

        const double ms = static_cast<double>(
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()) / 1000.0;
        metrics_.histogram(duration_, ms);

        if (!result.is_ok())
            metrics_.increment(errors_);

        return result;
    }

private:
    UseCase<In, Out, Err>& inner_;
    Metrics&               metrics_;
    std::string            calls_;
    std::string            errors_;
    std::string            duration_;
};

// Deduction guide so callers don't need to spell out template args.
template <typename In, typename Out, typename Err, kernel::MetricsPort Metrics>
InstrumentedUseCase(UseCase<In, Out, Err>&, Metrics&, std::string)
    -> InstrumentedUseCase<In, Out, Err, Metrics>;

} // namespace cpp_commons::application
