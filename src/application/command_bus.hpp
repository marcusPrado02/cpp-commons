#pragma once
#include <any>
#include <functional>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace cpp_commons::application {

struct CommandNotRegistered : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// Type-erased synchronous command bus.
// Register a handler with register_handler<Cmd>(); dispatch via send().
class CommandBus {
public:
    template <typename Cmd, typename Handler>
    void register_handler(Handler handler) {
        handlers_[std::type_index(typeid(Cmd))] =
            [h = std::move(handler)](const std::any& cmd) {
                h(std::any_cast<const Cmd&>(cmd));
            };
    }

    template <typename Cmd>
    void send(const Cmd& cmd) {
        auto it = handlers_.find(std::type_index(typeid(Cmd)));
        if (it == handlers_.end())
            throw CommandNotRegistered{"no handler registered for command"};
        it->second(cmd);
    }

private:
    std::unordered_map<std::type_index, std::function<void(const std::any&)>> handlers_;
};

} // namespace cpp_commons::application
