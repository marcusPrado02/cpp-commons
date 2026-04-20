#pragma once
#include <cstdint>
#include <vector>

namespace cpp_commons::application {

struct PageRequest {
    uint32_t page{0};   // 0-indexed
    uint32_t size{20};
    [[nodiscard]] uint32_t offset() const noexcept { return page * size; }
};

template <typename T>
struct Page {
    std::vector<T> items;
    uint64_t total{0};
    uint32_t page{0};
    uint32_t size{0};

    [[nodiscard]] bool has_next() const noexcept {
        return static_cast<uint64_t>((page + 1)) * size < total;
    }
    [[nodiscard]] uint32_t total_pages() const noexcept {
        if (size == 0) return 0;
        return static_cast<uint32_t>((total + size - 1) / size);
    }
};

} // namespace cpp_commons::application
