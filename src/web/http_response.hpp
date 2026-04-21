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
        return {pd.status, {{"Content-Type", "application/problem+json"}}, pd.to_json().dump()};
    }

    [[nodiscard]] static HttpResponse bad_request(std::string detail) {
        return from_problem(errors::ProblemDetails::validation_error(std::move(detail)));
    }

    [[nodiscard]] static HttpResponse unauthorized() {
        return from_problem(errors::ProblemDetails::unauthorized());
    }

    [[nodiscard]] static HttpResponse not_found(std::string detail) {
        return from_problem(errors::ProblemDetails::not_found(std::move(detail)));
    }

    [[nodiscard]] static HttpResponse conflict(std::string detail) {
        return from_problem(errors::ProblemDetails::conflict(std::move(detail)));
    }

    [[nodiscard]] static HttpResponse internal_error() {
        return from_problem(errors::ProblemDetails::internal_error());
    }
};

} // namespace cpp_commons::web
