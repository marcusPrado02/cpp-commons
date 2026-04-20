#pragma once
#include <concepts>
#include <optional>

namespace cpp_commons::kernel {

template<typename Repo, typename T, typename TId>
concept RepositoryPort = requires(Repo r, const T& entity, const TId& id) {
    { r.save(entity)   } -> std::same_as<void>;
    { r.find_by_id(id) } -> std::same_as<std::optional<T>>;
    { r.remove(id)     } -> std::same_as<void>;
};

} // namespace cpp_commons::kernel
