#pragma once

namespace cpp_commons::kernel {

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
