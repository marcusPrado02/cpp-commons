/// @file domain_error.hpp
/// @brief Exception hierarchy: DomainError / ApplicationError / InfrastructureError.
#pragma once
#include <stdexcept>
#include <string>

namespace cpp_commons::errors {

/// @brief Base for business-rule violations (entity invariants, aggregate constraints).
class DomainError : public std::runtime_error {
public:
    explicit DomainError(std::string message)
        : std::runtime_error{message}, message_{std::move(message)} {}
    [[nodiscard]] const std::string& message() const noexcept { return message_; }

private:
    std::string message_;
};

class NotFoundError final : public DomainError {
public:
    using DomainError::DomainError;
};

class ConflictError final : public DomainError {
public:
    using DomainError::DomainError;
};

class InvariantViolationError final : public DomainError {
public:
    using DomainError::DomainError;
};

/// @brief Base for application-layer errors (validation, auth, rate-limiting).
class ApplicationError : public std::runtime_error {
public:
    explicit ApplicationError(std::string message)
        : std::runtime_error{message}, message_{std::move(message)} {}
    [[nodiscard]] const std::string& message() const noexcept { return message_; }

private:
    std::string message_;
};

class ValidationError final : public ApplicationError {
public:
    using ApplicationError::ApplicationError;
};

class UnauthorizedError final : public ApplicationError {
public:
    using ApplicationError::ApplicationError;
};

class ForbiddenError final : public ApplicationError {
public:
    using ApplicationError::ApplicationError;
};

class RateLimitError final : public ApplicationError {
public:
    using ApplicationError::ApplicationError;
};

class TimeoutError final : public ApplicationError {
public:
    using ApplicationError::ApplicationError;
};

/// @brief Base for infrastructure errors (DB down, external service failure).
class InfrastructureError : public std::runtime_error {
public:
    explicit InfrastructureError(std::string message)
        : std::runtime_error{message}, message_{std::move(message)} {}
    [[nodiscard]] const std::string& message() const noexcept { return message_; }

private:
    std::string message_;
};

class ExternalServiceError final : public InfrastructureError {
public:
    using InfrastructureError::InfrastructureError;
};

}  // namespace cpp_commons::errors
