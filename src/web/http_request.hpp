#pragma once
#include <string>
#include <unordered_map>
#include <optional>

namespace cpp_commons::web {

enum class HttpMethod { Get, Post, Put, Patch, Delete, Head, Options };

using Headers = std::unordered_map<std::string, std::string>;

struct HttpRequest {
    HttpMethod method{HttpMethod::Get};
    std::string path;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> query_params;
    std::string body;

    [[nodiscard]] std::optional<std::string> header(const std::string& name) const {
        auto it = headers.find(name);
        if (it == headers.end()) return std::nullopt;
        return it->second;
    }

    [[nodiscard]] std::optional<std::string> param(const std::string& name) const {
        auto it = query_params.find(name);
        if (it == query_params.end()) return std::nullopt;
        return it->second;
    }
};

} // namespace cpp_commons::web
