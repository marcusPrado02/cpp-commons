#pragma once
#include <cpp_commons/kernel/ports/repository_port.hpp>
#include <optional>
#include <unordered_map>
#include <stdexcept>

namespace cpp_commons::testing {

// In-memory repository for use in unit tests.
// TId must be hashable (provide std::hash<TId> or use a string-based ID).
template <typename T, typename TId>
class FakeRepository {
public:
    void save(const T& entity) {
        store_[entity.id()] = entity;
    }

    [[nodiscard]] std::optional<T> find_by_id(const TId& id) const {
        auto it = store_.find(id);
        if (it == store_.end()) return std::nullopt;
        return it->second;
    }

    void remove(const TId& id) {
        store_.erase(id);
    }

    [[nodiscard]] std::size_t size() const noexcept { return store_.size(); }
    [[nodiscard]] bool empty()       const noexcept { return store_.empty(); }
    void clear() noexcept { store_.clear(); }

private:
    std::unordered_map<TId, T> store_;
};

} // namespace cpp_commons::testing
