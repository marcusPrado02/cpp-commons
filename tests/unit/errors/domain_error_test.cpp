#include <cpp_commons/errors/domain_error.hpp>
#include <gtest/gtest.h>
#include <stdexcept>

using namespace cpp_commons::errors;

TEST(DomainErrorTest, NotFoundIsThrowable) {
    EXPECT_THROW(throw NotFoundError{"order 42 not found"}, DomainError);
}

TEST(DomainErrorTest, NotFoundMessageAccessible) {
    NotFoundError e{"order 42 not found"};
    EXPECT_EQ(e.message(), "order 42 not found");
}

TEST(DomainErrorTest, ConflictIsDomainError) {
    ConflictError e{"duplicate key"};
    EXPECT_EQ(e.message(), "duplicate key");
}

TEST(DomainErrorTest, ValidationIsApplicationError) {
    ValidationError e{"field required"};
    EXPECT_EQ(e.message(), "field required");
    EXPECT_THROW(throw e, ApplicationError);
}

TEST(DomainErrorTest, UnauthorizedIsApplicationError) {
    UnauthorizedError e{"token expired"};
    EXPECT_THROW(throw e, ApplicationError);
}

TEST(DomainErrorTest, ExternalServiceIsInfrastructureError) {
    ExternalServiceError e{"payment gateway timeout"};
    EXPECT_THROW(throw e, InfrastructureError);
}
