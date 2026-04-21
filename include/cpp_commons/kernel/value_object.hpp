/// @file value_object.hpp
/// @brief CRTP base for immutable DDD value objects with structural equality.
#pragma once

namespace cpp_commons::kernel {

/// @brief CRTP base for DDD value objects.
///
/// Derived class must expose `auto fields() const` returning a comparable tuple.
/// Copy-assign is deleted; equality is delegated to `fields()` comparison.
// CRTP base — derived class must expose `fields()` returning a tuple.
// Provides value-based equality and immutability (no copy-assign).
template<typename Derived>
class ValueObject {
public:
    bool operator==(const ValueObject& other) const noexcept {
        return derived().fields() == static_cast<const Derived&>(other).fields();
    }
    bool operator!=(const ValueObject& other) const noexcept {
        return !(*this == other);
    }

protected:
    ValueObject() = default;
    ValueObject(const ValueObject&) = default;
    ValueObject& operator=(const ValueObject&) = delete;  // immutable
    ~ValueObject() = default;

private:
    const Derived& derived() const { return static_cast<const Derived&>(*this); }
};

} // namespace cpp_commons::kernel
