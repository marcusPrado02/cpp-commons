#pragma once
#include "http_request.hpp"
#include "http_response.hpp"

#include <functional>
#include <vector>

namespace cpp_commons::web {

using Handler = std::function<HttpResponse(const HttpRequest&)>;
using Middleware = std::function<HttpResponse(const HttpRequest&, const Handler&)>;

// Composes a list of middleware around a final handler.
// Middlewares execute in registration order; each receives a next() callable
// pointing to the remaining chain.
class MiddlewareChain {
public:
    MiddlewareChain& use(Middleware mw) {
        middlewares_.push_back(std::move(mw));
        return *this;
    }

    [[nodiscard]] HttpResponse dispatch(const HttpRequest& req,
                                        const Handler& final_handler) const {
        return build_chain(0, final_handler)(req);
    }

private:
    std::vector<Middleware> middlewares_;

    [[nodiscard]] Handler build_chain(std::size_t idx, const Handler& final_handler) const {
        if (idx >= middlewares_.size())
            return final_handler;
        const Middleware& mw = middlewares_[idx];
        Handler next = build_chain(idx + 1, final_handler);
        return [&mw, next](const HttpRequest& r) { return mw(r, next); };
    }
};

}  // namespace cpp_commons::web
