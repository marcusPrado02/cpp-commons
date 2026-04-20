#pragma once
#include "domain_event.hpp"
#include "entity.hpp"
#include <memory>
#include <utility>
#include <vector>

namespace cpp_commons::kernel {

template<EntityIdentifier TId>
class AggregateRoot : public Entity<TId> {
public:
    [[nodiscard]] bool has_pending_events() const noexcept {
        return !pending_events_.empty();
    }

    [[nodiscard]] std::vector<std::unique_ptr<DomainEvent>> pull_events() {
        return std::exchange(pending_events_, {});
    }

protected:
    using Entity<TId>::Entity;

    void record(std::unique_ptr<DomainEvent> event) {
        pending_events_.push_back(std::move(event));
    }

    template<typename E, typename... Args>
    void record(Args&&... args) {
        pending_events_.push_back(std::make_unique<E>(std::forward<Args>(args)...));
    }

private:
    std::vector<std::unique_ptr<DomainEvent>> pending_events_;
};

} // namespace cpp_commons::kernel
