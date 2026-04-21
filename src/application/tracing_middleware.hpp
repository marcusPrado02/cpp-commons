#pragma once
#include "command_bus.hpp"
#include "query_bus.hpp"

#include <typeinfo>

#include <cpp_commons/kernel/ports/tracer_port.hpp>

namespace cpp_commons::application {

// Wraps a CommandBus handler with a tracing span.
template <kernel::TracerPort Tracer, typename Cmd, typename Handler>
auto tracing_command_handler(Tracer& tracer, Handler handler) {
    return [&tracer, h = std::move(handler)](const Cmd& cmd) {
        tracer.start_span(typeid(Cmd).name());
        h(cmd);
        tracer.end_span();
    };
}

// Wraps a QueryBus handler with a tracing span.
template <kernel::TracerPort Tracer, typename Query, typename Result, typename Handler>
auto tracing_query_handler(Tracer& tracer, Handler handler) {
    return [&tracer, h = std::move(handler)](const Query& q) -> Result {
        tracer.start_span(typeid(Query).name());
        auto result = h(q);
        tracer.end_span();
        return result;
    };
}

}  // namespace cpp_commons::application
