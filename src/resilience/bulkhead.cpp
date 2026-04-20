#include "bulkhead.hpp"

namespace cpp_commons::resilience {

Bulkhead::Bulkhead(uint32_t max_concurrent) : max_(max_concurrent) {}

uint32_t Bulkhead::available() const noexcept {
    std::lock_guard lk{mu_};
    return max_ > active_ ? max_ - active_ : 0;
}

void Bulkhead::acquire() {
    std::lock_guard lk{mu_};
    if (active_ >= max_) throw BulkheadFullError{"bulkhead capacity reached"};
    ++active_;
}

void Bulkhead::release() noexcept {
    std::lock_guard lk{mu_};
    if (active_ > 0) --active_;
}

} // namespace cpp_commons::resilience
