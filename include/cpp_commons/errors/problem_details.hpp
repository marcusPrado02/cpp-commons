#pragma once
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
        return {"about:blank", "Not Found", 404, std::move(detail), ""};
    }
    static ProblemDetails conflict(std::string detail) {
        return {"about:blank", "Conflict", 409, std::move(detail), ""};
    }
    static ProblemDetails validation_error(std::string detail) {
        return {"about:blank", "Unprocessable Entity", 422, std::move(detail), ""};
    }
    static ProblemDetails unauthorized() {
        return {"about:blank", "Unauthorized", 401, "Authentication required", ""};
    }
    static ProblemDetails internal_error() {
        return {"about:blank", "Internal Server Error", 500, "An unexpected error occurred", ""};
    }
};

} // namespace cpp_commons::errors
