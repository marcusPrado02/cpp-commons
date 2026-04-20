#pragma once
#include <cpp_commons/errors/problem_details.hpp>
#include <string>
#include <unordered_map>

namespace cpp_commons::web {

struct HttpResponse {
    int status_code{200};
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    std::string content_type{"application/json"};

    [[nodiscard]] static HttpResponse ok(std::string body) {
        return {200, {}, std::move(body)};
    }

    [[nodiscard]] static HttpResponse created(std::string body) {
        return {201, {}, std::move(body)};
    }

    [[nodiscard]] static HttpResponse no_content() {
        return {204, {}, ""};
    }

    [[nodiscard]] static HttpResponse from_problem(const errors::ProblemDetails& pd) {
        return {pd.status, {}, pd.to_json().dump()};
    }
};

} // namespace cpp_commons::web
