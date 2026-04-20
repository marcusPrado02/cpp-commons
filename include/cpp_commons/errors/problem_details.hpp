#pragma once
#include "error_codes.hpp"
#include <nlohmann/json.hpp>
#include <string>

namespace cpp_commons::errors {

struct ProblemDetails {
    std::string type;
    std::string title;
    int         status{500};
    std::string detail;
    std::string instance;

    [[nodiscard]] nlohmann::json to_json() const {
        return {
            {"type",     type},
            {"title",    title},
            {"status",   status},
            {"detail",   detail},
            {"instance", instance},
        };
    }

    static ProblemDetails not_found(std::string detail) {
        return {std::string{error_type::not_found}, "Not Found",
                http_status::not_found, std::move(detail), ""};
    }
    static ProblemDetails conflict(std::string detail) {
        return {std::string{error_type::conflict}, "Conflict",
                http_status::conflict, std::move(detail), ""};
    }
    static ProblemDetails validation_error(std::string detail) {
        return {std::string{error_type::validation}, "Unprocessable Entity",
                http_status::unprocessable_entity, std::move(detail), ""};
    }
    static ProblemDetails unauthorized() {
        return {std::string{error_type::unauthorized}, "Unauthorized",
                http_status::unauthorized, "Authentication required", ""};
    }
    static ProblemDetails internal_error() {
        return {std::string{error_type::internal}, "Internal Server Error",
                http_status::internal_server_error, "An unexpected error occurred", ""};
    }
};

} // namespace cpp_commons::errors
