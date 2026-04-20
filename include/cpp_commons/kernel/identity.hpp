#pragma once
#include <array>
#include <cstdint>
#include <format>
#include <functional>
#include <random>
#include <string>

namespace cpp_commons::kernel {

class UUID {
public:
    static UUID generate() {
        static thread_local std::mt19937_64 rng{std::random_device{}()};
        std::uniform_int_distribution<uint64_t> dist;
        uint64_t a = dist(rng);
        uint64_t b = dist(rng);
        a = (a & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;  // version 4
        b = (b & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;  // variant
        return UUID{a, b};
    }

    std::string to_string() const {
        return std::format("{:08x}-{:04x}-{:04x}-{:04x}-{:012x}",
            static_cast<uint32_t>(hi_ >> 32),
            static_cast<uint16_t>(hi_ >> 16),
            static_cast<uint16_t>(hi_),
            static_cast<uint16_t>(lo_ >> 48),
            lo_ & 0x0000FFFFFFFFFFFFULL);
    }

    [[nodiscard]] uint64_t hi() const noexcept { return hi_; }
    [[nodiscard]] uint64_t lo() const noexcept { return lo_; }

    bool operator==(const UUID&) const = default;
    bool operator<(const UUID& o)  const { return hi_ != o.hi_ ? hi_ < o.hi_ : lo_ < o.lo_; }

private:
    UUID(uint64_t hi, uint64_t lo) : hi_{hi}, lo_{lo} {}
    uint64_t hi_{};
    uint64_t lo_{};
};

template<typename Tag>
class StrongId {
public:
    StrongId() : uuid_{UUID::generate()} {}
    explicit StrongId(UUID uuid) : uuid_{std::move(uuid)} {}

    const UUID& uuid()         const { return uuid_; }
    std::string to_string()    const { return uuid_.to_string(); }

    bool operator==(const StrongId&) const = default;
    bool operator<(const StrongId& o) const { return uuid_ < o.uuid_; }

private:
    UUID uuid_;
};

struct EntityIdTag    {};
struct CorrelationIdTag {};
struct TenantIdTag    {};
struct RequestIdTag   {};

using EntityId      = StrongId<EntityIdTag>;
using CorrelationId = StrongId<CorrelationIdTag>;
using TenantId      = StrongId<TenantIdTag>;
using RequestId     = StrongId<RequestIdTag>;

} // namespace cpp_commons::kernel

// std::hash specializations — enable use in unordered_map/unordered_set
template<>
struct std::hash<cpp_commons::kernel::UUID> {
    std::size_t operator()(const cpp_commons::kernel::UUID& u) const noexcept {
        // Knuth multiplicative hash combiner — better distribution than plain XOR
        return std::hash<uint64_t>{}(u.hi()) ^
               (std::hash<uint64_t>{}(u.lo()) * 0x9e3779b97f4a7c15ULL);
    }
};

template<typename Tag>
struct std::hash<cpp_commons::kernel::StrongId<Tag>> {
    std::size_t operator()(const cpp_commons::kernel::StrongId<Tag>& id) const noexcept {
        return std::hash<cpp_commons::kernel::UUID>{}(id.uuid());
    }
};
