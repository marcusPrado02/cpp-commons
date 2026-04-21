#pragma once
#include "http_request.hpp"
#include "http_response.hpp"
#include "middleware_chain.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace cpp_commons::web {

// Parses the Accept header and returns the best matching media type
// from the provided list. Returns "" if no match found.
// Respects quality factors (q=) and uses simple prefix matching.
[[nodiscard]] inline std::string negotiate_content_type(std::string_view accept_header,
                                                        const std::vector<std::string>& supported) {
    if (accept_header.empty() || accept_header == "*/*")
        return supported.empty() ? "" : supported.front();

    // Split accept header by comma, strip quality factors.
    struct AcceptEntry {
        std::string type;
        double q;
    };
    std::vector<AcceptEntry> entries;

    std::size_t pos = 0;
    while (pos < accept_header.size()) {
        auto comma = accept_header.find(',', pos);
        auto token = accept_header.substr(
            pos, comma == std::string_view::npos ? std::string_view::npos : comma - pos);
        pos = comma == std::string_view::npos ? accept_header.size() : comma + 1;

        // Trim whitespace
        while (!token.empty() && token.front() == ' ')
            token.remove_prefix(1);
        while (!token.empty() && token.back() == ' ')
            token.remove_suffix(1);

        double q = 1.0;
        auto semi = token.find(';');
        if (semi != std::string_view::npos) {
            auto param = token.substr(semi + 1);
            while (!param.empty() && param.front() == ' ')
                param.remove_prefix(1);
            if (param.substr(0, 2) == "q=") {
                try {
                    q = std::stod(std::string{param.substr(2)});
                } catch (...) {
                }
            }
            token = token.substr(0, semi);
        }
        entries.push_back({std::string{token}, q});
    }

    // Sort by quality descending.
    std::sort(entries.begin(), entries.end(),
              [](const AcceptEntry& a, const AcceptEntry& b) { return a.q > b.q; });

    for (const auto& entry : entries) {
        for (const auto& sup : supported) {
            if (entry.type == "*/*" || entry.type == sup)
                return sup;
            // Wildcard subtype: "application/*" matches "application/json"
            auto slash = entry.type.find('/');
            if (slash != std::string::npos && entry.type.substr(slash + 1) == "*") {
                auto sup_slash = sup.find('/');
                if (sup_slash != std::string::npos &&
                    entry.type.substr(0, slash) == sup.substr(0, sup_slash))
                    return sup;
            }
        }
    }
    return "";
}

// Middleware: performs content negotiation based on the Accept header.
// If no match is found, returns 406 Not Acceptable.
// Sets response Content-Type to the negotiated type.
inline Middleware content_negotiation_middleware(std::vector<std::string> supported = {
                                                     "application/json"}) {
    return [sup = std::move(supported)](const HttpRequest& req,
                                        const Handler& next) -> HttpResponse {
        auto accept = req.header("Accept").value_or("*/*");
        auto content_type = negotiate_content_type(accept, sup);

        if (content_type.empty()) {
            return HttpResponse{406,
                                {{"Content-Type", "text/plain"}},
                                "Not Acceptable: no supported media type matches Accept header"};
        }

        auto resp = next(req);
        if (resp.headers.find("Content-Type") == resp.headers.end())
            resp.headers["Content-Type"] = content_type;
        return resp;
    };
}

}  // namespace cpp_commons::web
