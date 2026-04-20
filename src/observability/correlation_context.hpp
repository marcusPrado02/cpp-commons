#pragma once
#include <cpp_commons/kernel/identity.hpp>
#include <string>

namespace cpp_commons::observability {

struct CorrelationContext {
    std::string correlation_id;
    std::string tenant_id;
    std::string trace_id;
    std::string request_id;

    [[nodiscard]] static CorrelationContext generate() {
        return {
            kernel::UUID::generate().to_string(),
            "",
            kernel::UUID::generate().to_string(),
            kernel::UUID::generate().to_string(),
        };
    }

    [[nodiscard]] bool empty() const noexcept { return correlation_id.empty(); }
};

// Returns the context installed by the innermost active CorrelationScope.
[[nodiscard]] const CorrelationContext& current_correlation() noexcept;

// RAII guard: installs ctx as current, restores previous on destruction.
class CorrelationScope {
public:
    explicit CorrelationScope(CorrelationContext ctx);
    ~CorrelationScope();
    CorrelationScope(const CorrelationScope&) = delete;
    CorrelationScope& operator=(const CorrelationScope&) = delete;
    CorrelationScope(CorrelationScope&&) = delete;
    CorrelationScope& operator=(CorrelationScope&&) = delete;

private:
    CorrelationContext previous_;
};

} // namespace cpp_commons::observability
