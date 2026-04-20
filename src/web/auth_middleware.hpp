#pragma once
#include "middleware_chain.hpp"
#include <functional>
#include <string>

namespace cpp_commons::web {

// Extracts a Bearer token from the Authorization header and calls verify_fn.
// verify_fn returns true if the token is valid; on failure returns 401.
inline Middleware auth_middleware(std::function<bool(const std::string&)> verify_fn) {
    return [vfn = std::move(verify_fn)](const HttpRequest& req, const Handler& next) -> HttpResponse {
        auto auth = req.header("Authorization");
        if (!auth || auth->size() < 8 || auth->substr(0, 7) != "Bearer ") {
            return HttpResponse{401, {}, R"({"error":"missing or invalid Authorization header"})"};
        }
        const std::string token = auth->substr(7);
        if (!vfn(token)) {
            return HttpResponse{401, {}, R"({"error":"token verification failed"})"};
        }
        return next(req);
    };
}

} // namespace cpp_commons::web
