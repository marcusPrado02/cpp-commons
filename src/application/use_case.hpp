#pragma once
#include <cpp_commons/kernel/result.hpp>

namespace cpp_commons::application {

// Base interface for application use-cases.
// Concrete classes implement execute() and inject dependencies in the ctor.
template <typename In, typename Out, typename Err>
struct UseCase {
    using Input  = In;
    using Output = Out;
    using Error  = Err;
    using Result = kernel::Result<Out, Err>;

    virtual ~UseCase() = default;
    [[nodiscard]] virtual Result execute(const In& input) = 0;
};

} // namespace cpp_commons::application
