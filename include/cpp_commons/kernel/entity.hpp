/// @file entity.hpp
/// @brief DDD Entity base — identity-based equality via `EntityIdentifier` concept.
#pragma once
#include <concepts>
#include <utility>

namespace cpp_commons::kernel {

/// @brief Constraint: any equality-comparable type may serve as an entity identifier.
template<typename T>
concept EntityIdentifier = std::equality_comparable<T>;

/// @brief DDD Entity whose identity is determined solely by its `id()`, not its attributes.
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
