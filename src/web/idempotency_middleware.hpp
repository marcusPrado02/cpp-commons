#pragma once
#include "http_request.hpp"
#include "http_response.hpp"
#include "middleware_chain.hpp"

#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace cpp_commons::web {

// In-memory idempotency cache entry.
struct IdempotencyEntry {
    HttpResponse response;
    std::chrono::steady_clock::time_point expires_at;
};

// Idempotency middleware: caches responses keyed by the Idempotency-Key header.
// Repeat requests with the same key within the TTL return the cached response
// without invoking the handler again.
//
// Uses the "Idempotency-Key" request header (Stripe-compatible).
// Requests without the header pass through unchanged.
inline Middleware idempotency_middleware(std::chrono::seconds ttl = std::chrono::seconds{3600}) {
    struct Cache {
        std::unordered_map<std::string, IdempotencyEntry> entries;
        std::mutex mutex;
    };
    auto cache = std::make_shared<Cache>();

    return [cache, ttl](const HttpRequest& req, const Handler& next) -> HttpResponse {
        auto it = req.headers.find("Idempotency-Key");
        if (it == req.headers.end() || it->second.empty())
            return next(req);

        const std::string& key = it->second;
        auto now = std::chrono::steady_clock::now();

        {
            std::lock_guard lock{cache->mutex};
            auto cached = cache->entries.find(key);
            if (cached != cache->entries.end()) {
                if (cached->second.expires_at > now)
                    return cached->second.response;
                cache->entries.erase(cached);  // expired
            }
        }

        auto response = next(req);

        {
            std::lock_guard lock{cache->mutex};
            cache->entries.insert_or_assign(key, IdempotencyEntry{
                                                     response,
                                                     now + ttl,
                                                 });
        }
        return response;
    };
}

}  // namespace cpp_commons::web
