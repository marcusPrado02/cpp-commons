/// @file identity.hpp
/// @brief RFC 4122 v4 UUID and tag-dispatched StrongId with std::hash support.
#pragma once
#include <array>
#include <charconv>
#include <cstdint>
#include <format>
#include <functional>
#include <optional>
#include <random>
#include <string>
#include <string_view>

namespace cpp_commons::kernel {

/// @brief Randomly-generated RFC 4122 version-4 UUID.
///
/// Two halves stored as uint64_t. Use `generate()` for new ids, `from_string()`
/// to deserialise from HTTP/JSON/DB. `std::hash` specialisation provided.
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

    // Parse "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx". Returns nullopt on any error.
    [[nodiscard]] static std::optional<UUID> from_string(std::string_view s) noexcept {
        // Fixed layout: 8-4-4-4-12 hex digits + 4 dashes = 36 chars
        if (s.size() != 36)
            return std::nullopt;
        if (s[8] != '-' || s[13] != '-' || s[18] != '-' || s[23] != '-')
            return std::nullopt;

        auto read_hex = [](std::string_view src, uint64_t& out) -> bool {
            uint64_t v = 0;
            for (char c : src) {
                v <<= 4;
                if (c >= '0' && c <= '9')
                    v |= static_cast<uint64_t>(c - '0');
                else if (c >= 'a' && c <= 'f')
                    v |= static_cast<uint64_t>(c - 'a' + 10);
                else if (c >= 'A' && c <= 'F')
                    v |= static_cast<uint64_t>(c - 'A' + 10);
                else
                    return false;
            }
            out = v;
            return true;
        };

        uint64_t p1{}, p2{}, p3{}, p4{}, p5{};
        if (!read_hex(s.substr(0, 8), p1))
            return std::nullopt;
        if (!read_hex(s.substr(9, 4), p2))
            return std::nullopt;
        if (!read_hex(s.substr(14, 4), p3))
            return std::nullopt;
        if (!read_hex(s.substr(19, 4), p4))
            return std::nullopt;
        if (!read_hex(s.substr(24, 12), p5))
            return std::nullopt;

        uint64_t hi = (p1 << 32) | (p2 << 16) | p3;
        uint64_t lo = (p4 << 48) | p5;
        return UUID{hi, lo};
    }

    [[nodiscard]] std::string to_string() const {
        return std::format("{:08x}-{:04x}-{:04x}-{:04x}-{:012x}", static_cast<uint32_t>(hi_ >> 32),
                           static_cast<uint16_t>(hi_ >> 16), static_cast<uint16_t>(hi_),
                           static_cast<uint16_t>(lo_ >> 48), lo_ & 0x0000FFFFFFFFFFFFULL);
    }

    // 32-char lowercase hex, no dashes — W3C Trace Context trace-id format.
    [[nodiscard]] std::string to_string_no_dashes() const {
        return std::format("{:016x}{:016x}", hi_, lo_);
    }

    [[nodiscard]] uint64_t hi() const noexcept { return hi_; }
    [[nodiscard]] uint64_t lo() const noexcept { return lo_; }

    bool operator==(const UUID&) const = default;
    bool operator<(const UUID& o) const { return hi_ != o.hi_ ? hi_ < o.hi_ : lo_ < o.lo_; }

private:
    UUID(uint64_t hi, uint64_t lo) : hi_{hi}, lo_{lo} {}
    uint64_t hi_{};
    uint64_t lo_{};
};

/// @brief Tag-dispatched UUID wrapper preventing implicit cross-domain id mixing.
///
/// `StrongId<OrderTag>` and `StrongId<UserId>` are different types at compile
/// time even though both hold a UUID internally.
template <typename Tag>
class StrongId {
public:
    StrongId() : uuid_{UUID::generate()} {}
    explicit StrongId(UUID uuid) : uuid_{uuid} {}

    // Parse from string representation — returns nullopt if invalid.
    [[nodiscard]] static std::optional<StrongId> from_string(std::string_view s) noexcept {
        auto uuid = UUID::from_string(s);
        if (!uuid)
            return std::nullopt;
        return StrongId{*uuid};
    }

    [[nodiscard]] const UUID& uuid() const noexcept { return uuid_; }
    [[nodiscard]] std::string to_string() const { return uuid_.to_string(); }

    bool operator==(const StrongId&) const = default;
    bool operator<(const StrongId& o) const { return uuid_ < o.uuid_; }

private:
    UUID uuid_;
};

struct EntityIdTag {};
struct CorrelationIdTag {};
struct TenantIdTag {};
struct RequestIdTag {};

using EntityId = StrongId<EntityIdTag>;
using CorrelationId = StrongId<CorrelationIdTag>;
using TenantId = StrongId<TenantIdTag>;
using RequestId = StrongId<RequestIdTag>;

}  // namespace cpp_commons::kernel

// std::hash specializations — enable use in unordered_map/unordered_set
template <>
struct std::hash<cpp_commons::kernel::UUID> {
    std::size_t operator()(const cpp_commons::kernel::UUID& u) const noexcept {
        return std::hash<uint64_t>{}(u.hi()) ^
               (std::hash<uint64_t>{}(u.lo()) * 0x9e3779b97f4a7c15ULL);
    }
};

template <typename Tag>
struct std::hash<cpp_commons::kernel::StrongId<Tag>> {
    std::size_t operator()(const cpp_commons::kernel::StrongId<Tag>& id) const noexcept {
        return std::hash<cpp_commons::kernel::UUID>{}(id.uuid());
    }
};
