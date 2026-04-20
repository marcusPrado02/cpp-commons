#pragma once
#include <concepts>
#include <utility>

namespace cpp_commons::kernel {

template<typename T>
concept EntityIdentifier = std::equality_comparable<T>;

template<EntityIdentifier TId>
class Entity {
public:
    using id_type = TId;

    [[nodiscard]] const TId& id() const noexcept { return id_; }

    bool operator==(const Entity& other) const noexcept { return id_ == other.id_; }
    bool operator!=(const Entity& other) const noexcept { return !(*this == other); }

protected:
    explicit Entity(TId id) : id_{std::move(id)} {}
    Entity(const Entity&) = default;
    Entity& operator=(const Entity&) = default;
    Entity(Entity&&) = default;
    Entity& operator=(Entity&&) = default;
    virtual ~Entity() = default;

private:
    TId id_;
};

} // namespace cpp_commons::kernel
