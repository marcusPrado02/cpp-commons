#include "api_key.hpp"
#include <stdexcept>

namespace cpp_commons::security {

ApiKey::ApiKey(std::string raw) : raw_(std::move(raw)) {
    if (raw_.empty()) throw std::invalid_argument{"ApiKey must not be empty"};
}

bool ApiKey::operator==(const ApiKey& other) const noexcept {
    if (raw_.size() != other.raw_.size()) return false;
    // Constant-time comparison: accumulate differences without early exit.
    unsigned char diff = 0;
    for (std::size_t i = 0; i < raw_.size(); ++i) {
        diff = static_cast<unsigned char>(diff |
               (static_cast<unsigned char>(raw_[i]) ^
                static_cast<unsigned char>(other.raw_[i])));
    }
    return diff == 0;
}

bool ApiKey::operator!=(const ApiKey& other) const noexcept {
    return !(*this == other);
}

} // namespace cpp_commons::security
