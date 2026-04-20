#pragma once
#include <cpp_commons/errors/domain_error.hpp>
#include <functional>
#include <stdexcept>
#include <string>

namespace cpp_commons::application {

struct ValidationError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// Wraps a CommandBus handler with a validation predicate.
// Throws ValidationError (not cpp_commons::errors::ValidationError) so the bus
// propagates it as an unhandled exception, letting the caller decide.
template <typename Cmd, typename Handler, typename Validator>
auto validating_command_handler(Validator validator, Handler handler) {
    return [v = std::move(validator), h = std::move(handler)](const Cmd& cmd) {
        if (auto err = v(cmd); !err.empty())
            throw ValidationError{err};
        h(cmd);
    };
}

} // namespace cpp_commons::application
