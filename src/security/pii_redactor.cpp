#include "pii_redactor.hpp"

namespace cpp_commons::security {

namespace {
const std::regex kEmailRe{R"([a-zA-Z0-9._%+\-]+@[a-zA-Z0-9.\-]+\.[a-zA-Z]{2,})"};
const std::regex kCardRe{R"(\b(?:\d{4}[- ]?){3}\d{4}\b)"};
}

std::string PiiRedactor::redact_email(std::string_view input) {
    return std::regex_replace(std::string{input}, kEmailRe, "[EMAIL]");
}

std::string PiiRedactor::redact_card(std::string_view input) {
    return std::regex_replace(std::string{input}, kCardRe, "[CARD]");
}

std::string PiiRedactor::redact_all(std::string_view input) {
    return redact_card(redact_email(input));
}

} // namespace cpp_commons::security
