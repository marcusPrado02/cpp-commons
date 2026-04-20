#pragma once
#include <string_view>

namespace cpp_commons::errors {

// Well-known Problem Detail type URIs (RFC 9457).
// Use these as the `type` field in ProblemDetails to give errors stable,
// linkable identities — preferable to the opaque "about:blank" default.
namespace error_type {

inline constexpr std::string_view not_found          = "urn:cpp-commons:error:not-found";
inline constexpr std::string_view conflict           = "urn:cpp-commons:error:conflict";
inline constexpr std::string_view validation         = "urn:cpp-commons:error:validation";
inline constexpr std::string_view unauthorized       = "urn:cpp-commons:error:unauthorized";
inline constexpr std::string_view forbidden          = "urn:cpp-commons:error:forbidden";
inline constexpr std::string_view rate_limited       = "urn:cpp-commons:error:rate-limited";
inline constexpr std::string_view timeout            = "urn:cpp-commons:error:timeout";
inline constexpr std::string_view invariant_violated = "urn:cpp-commons:error:invariant-violated";
inline constexpr std::string_view external_service   = "urn:cpp-commons:error:external-service";
inline constexpr std::string_view internal           = "urn:cpp-commons:error:internal";

} // namespace error_type

// HTTP status codes as named constants, eliminating magic numbers in handlers.
namespace http_status {

inline constexpr int ok                    = 200;
inline constexpr int created               = 201;
inline constexpr int no_content            = 204;
inline constexpr int bad_request           = 400;
inline constexpr int unauthorized          = 401;
inline constexpr int forbidden             = 403;
inline constexpr int not_found             = 404;
inline constexpr int conflict              = 409;
inline constexpr int unprocessable_entity  = 422;
inline constexpr int too_many_requests     = 429;
inline constexpr int internal_server_error = 500;
inline constexpr int bad_gateway           = 502;
inline constexpr int service_unavailable   = 503;
inline constexpr int gateway_timeout       = 504;

} // namespace http_status

} // namespace cpp_commons::errors
