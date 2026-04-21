#pragma once
#include "command_bus.hpp"
#include "query_bus.hpp"

namespace cpp_commons::application {

// Mediator — single entry point for both commands and queries.
// Decouples callers from the underlying buses; useful for injecting
// the same object across use-cases that mix reads and writes.
class Mediator {
public:
    template <typename Cmd, typename Handler>
    void register_command(Handler handler) {
        commands_.register_handler<Cmd>(std::move(handler));
    }

    template <typename Query, typename Handler>
    void register_query(Handler handler) {
        queries_.register_handler<Query>(std::move(handler));
    }

    template <typename Cmd>
    void send(const Cmd& cmd) {
        commands_.send(cmd);
    }

    template <typename Result, typename Query>
    [[nodiscard]] Result query(const Query& q) {
        return queries_.query<Result>(q);
    }

private:
    CommandBus commands_;
    QueryBus queries_;
};

}  // namespace cpp_commons::application
