#pragma once
#include <regex>
#include <string>

namespace cpp_commons::security {

// Redacts common PII patterns from log messages and strings.
// All methods return a new string; original is unchanged.
class PiiRedactor {
public:
    // Replaces email addresses with [EMAIL].
    [[nodiscard]] static std::string redact_email(std::string_view input);
    // Replaces credit card numbers (groups of 4 digits) with [CARD].
    [[nodiscard]] static std::string redact_card(std::string_view input);
    // Applies all redactions in sequence.
    [[nodiscard]] static std::string redact_all(std::string_view input);
};

} // namespace cpp_commons::security
