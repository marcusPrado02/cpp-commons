#pragma once
#include "middleware_chain.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include <string>
#include <unordered_map>

namespace cpp_commons::web {

// In-memory HTTP server for testing — no TCP socket.
// Register middleware with use() and routes with route(), then call dispatch().
class FakeHttpServer {
public:
    FakeHttpServer& use(Middleware mw) {
        chain_.use(std::move(mw));
        return *this;
    }

    FakeHttpServer& route(HttpMethod method, std::string path, Handler handler) {
        routes_[key(method, path)] = std::move(handler);
        return *this;
    }

    [[nodiscard]] HttpResponse dispatch(const HttpRequest& req) const {
        auto it = routes_.find(key(req.method, req.path));
        if (it == routes_.end())
            return HttpResponse{404, {}, "Not Found"};
        return chain_.dispatch(req, it->second);
    }

    [[nodiscard]] HttpResponse get(std::string path, Headers hdrs = {}) const {
        HttpRequest req;
        req.method  = HttpMethod::Get;
        req.path    = std::move(path);
        req.headers = std::move(hdrs);
        return dispatch(req);
    }

    [[nodiscard]] HttpResponse post(std::string path, std::string body,
                                    Headers hdrs = {}) const {
        HttpRequest req;
        req.method  = HttpMethod::Post;
        req.path    = std::move(path);
        req.headers = std::move(hdrs);
        req.body    = std::move(body);
        return dispatch(req);
    }

private:
    MiddlewareChain chain_;
    std::unordered_map<std::string, Handler> routes_;

    static std::string key(HttpMethod m, const std::string& path) {
        return std::to_string(static_cast<int>(m)) + ':' + path;
    }
};

} // namespace cpp_commons::web
