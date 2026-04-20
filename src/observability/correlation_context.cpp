#include "correlation_context.hpp"

namespace cpp_commons::observability {

namespace {
thread_local CorrelationContext tl_current{};
}

const CorrelationContext& current_correlation() noexcept {
    return tl_current;
}

CorrelationScope::CorrelationScope(CorrelationContext ctx)
    : previous_(tl_current) {
    tl_current = std::move(ctx);
}

CorrelationScope::~CorrelationScope() {
    tl_current = std::move(previous_);
}

} // namespace cpp_commons::observability
