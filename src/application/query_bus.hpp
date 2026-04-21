#pragma once
#include <any>
#include <functional>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>

namespace cpp_commons::application {

struct QueryNotRegistered : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// Type-erased synchronous query bus.
// Handlers return std::any; callers cast with query<Query, Result>().
class QueryBus {
public:
    template <typename Query, typename Handler>
    void register_handler(Handler handler) {
        handlers_[std::type_index(typeid(Query))] =
            [h = std::move(handler)](const std::any& q) -> std::any {
            return h(std::any_cast<const Query&>(q));
        };
    }

    template <typename Result, typename Query>
    [[nodiscard]] Result query(const Query& q) {
        auto it = handlers_.find(std::type_index(typeid(Query)));
        if (it == handlers_.end())
            throw QueryNotRegistered{"no handler registered for query"};
        return std::any_cast<Result>(it->second(q));
    }

private:
    std::unordered_map<std::type_index, std::function<std::any(const std::any&)>> handlers_;
};

}  // namespace cpp_commons::application
