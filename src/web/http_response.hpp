#pragma once
#include <cpp_commons/errors/domain_error.hpp>
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

    // Automatic mapping from domain/application error hierarchy to HTTP responses.
    [[nodiscard]] static HttpResponse from_error(const std::exception& e) {
        using namespace errors;
        if (dynamic_cast<const NotFoundError*>(&e))
            return not_found(e.what());
        if (dynamic_cast<const ValidationError*>(&e))
            return bad_request(e.what());
        if (dynamic_cast<const UnauthorizedError*>(&e))
            return unauthorized();
        if (dynamic_cast<const ForbiddenError*>(&e))
            return from_problem({std::string{error_type::forbidden}, "Forbidden",
                                 403, e.what(), ""});
        if (dynamic_cast<const ConflictError*>(&e))
            return conflict(e.what());
        if (dynamic_cast<const RateLimitError*>(&e))
            return from_problem({std::string{error_type::rate_limited}, "Too Many Requests",
                                 429, e.what(), ""});
        if (dynamic_cast<const TimeoutError*>(&e))
            return from_problem({std::string{error_type::timeout}, "Gateway Timeout",
                                 504, e.what(), ""});
        return internal_error();
    }
};

} // namespace cpp_commons::web
