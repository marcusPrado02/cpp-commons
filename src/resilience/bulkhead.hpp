#pragma once
#include <cstdint>
#include <mutex>
#include <stdexcept>

namespace cpp_commons::resilience {

struct BulkheadFullError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// Concurrency limiter — caps simultaneous executions of fn.
// Throws BulkheadFullError when the limit is already reached.
class Bulkhead {
public:
    explicit Bulkhead(uint32_t max_concurrent);

    template <typename Fn>
    auto call(Fn&& fn) -> decltype(fn()) {
        acquire();
        struct Guard {
            Bulkhead* self;
            ~Guard() { self->release(); }
        } guard{this};
        return fn();
    }

    [[nodiscard]] uint32_t available() const noexcept;

private:
    void acquire();
    void release() noexcept;

    const uint32_t max_;
    mutable std::mutex mu_;
    uint32_t active_{0};
};

}  // namespace cpp_commons::resilience
