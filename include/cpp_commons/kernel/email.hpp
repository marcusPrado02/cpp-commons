/// @file email.hpp
/// @brief Validated email address value object — construction only via `Email::parse()`.
#pragma once
#include "value_object.hpp"

#include <string>
#include <string_view>

#include <cpp_commons/errors/domain_error.hpp>
#include <cpp_commons/kernel/result.hpp>

namespace cpp_commons::kernel {

/// @brief RFC 5322-simplified email address.
///
/// `parse()` enforces `local@domain.tld` structure and rejects whitespace.
/// Returns `Result<Email, ValidationError>` so callers must handle the error path.
class Email : public ValueObject<Email> {
public:
    [[nodiscard]] static Result<Email, errors::ValidationError> parse(std::string_view input) {
        if (input.empty())
            return Result<Email, errors::ValidationError>::err(
                errors::ValidationError{"Email must not be empty"});

        auto at = input.find('@');
        if (at == std::string_view::npos || at == 0 || at == input.size() - 1)
            return Result<Email, errors::ValidationError>::err(
                errors::ValidationError{"Email must contain '@' with local and domain parts"});

        auto domain = input.substr(at + 1);
        if (domain.find('.') == std::string_view::npos)
            return Result<Email, errors::ValidationError>::err(
                errors::ValidationError{"Email domain must contain '.'"});

        // Reject obvious invalid chars (simplified RFC 5322)
        for (char c : input) {
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
                return Result<Email, errors::ValidationError>::err(
                    errors::ValidationError{"Email must not contain whitespace"});
        }

        return Result<Email, errors::ValidationError>::ok(Email{std::string{input}});
    }

    [[nodiscard]] const std::string& value() const noexcept { return address_; }
    [[nodiscard]] std::string to_string() const { return address_; }
    [[nodiscard]] auto fields() const noexcept { return std::tie(address_); }

private:
    explicit Email(std::string address) : address_{std::move(address)} {}
    std::string address_;
};

}  // namespace cpp_commons::kernel
