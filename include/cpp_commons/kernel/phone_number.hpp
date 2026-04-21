/// @file phone_number.hpp
/// @brief E.164 phone number value object — construction only via `PhoneNumber::parse()`.
#pragma once
#include "value_object.hpp"
#include <cpp_commons/kernel/result.hpp>
#include <cpp_commons/errors/domain_error.hpp>
#include <string>
#include <string_view>

namespace cpp_commons::kernel {

/// @brief E.164-format phone number (`+` followed by 7–15 digits).
///
/// `parse()` validates the format and returns `Result<PhoneNumber, ValidationError>`.
class PhoneNumber : public ValueObject<PhoneNumber> {
public:
    [[nodiscard]] static Result<PhoneNumber, errors::ValidationError>
    parse(std::string_view input) {
        if (input.empty() || input[0] != '+')
            return Result<PhoneNumber, errors::ValidationError>::err(
                errors::ValidationError{"Phone number must start with '+'"});

        auto digits = input.substr(1);
        if (digits.size() < 7 || digits.size() > 15)
            return Result<PhoneNumber, errors::ValidationError>::err(
                errors::ValidationError{"E.164 requires 7-15 digits after country code"});

        for (char c : digits) {
            if (c < '0' || c > '9')
                return Result<PhoneNumber, errors::ValidationError>::err(
                    errors::ValidationError{"Phone number must contain only digits after '+'"});
        }

        return Result<PhoneNumber, errors::ValidationError>::ok(
            PhoneNumber{std::string{input}});
    }

    [[nodiscard]] const std::string& value()     const noexcept { return number_; }
    [[nodiscard]] std::string        to_string() const { return number_; }
    [[nodiscard]] auto               fields()    const noexcept { return std::tie(number_); }

private:
    explicit PhoneNumber(std::string number) : number_{std::move(number)} {}
    std::string number_;
};

} // namespace cpp_commons::kernel
