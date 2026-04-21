#include <cpp_commons/errors/problem_details.hpp>

#include <gtest/gtest.h>

using cpp_commons::errors::ProblemDetails;

TEST(ProblemDetailsTest, NotFoundFactory) {
    auto pd = ProblemDetails::not_found("Order 42 does not exist");
    EXPECT_EQ(pd.status, 404);
    EXPECT_EQ(pd.title, "Not Found");
    EXPECT_EQ(pd.detail, "Order 42 does not exist");
}

TEST(ProblemDetailsTest, ConflictFactory) {
    auto pd = ProblemDetails::conflict("duplicate email");
    EXPECT_EQ(pd.status, 409);
}

TEST(ProblemDetailsTest, ValidationFactory) {
    auto pd = ProblemDetails::validation_error("name is required");
    EXPECT_EQ(pd.status, 422);
}

TEST(ProblemDetailsTest, UnauthorizedFactory) {
    auto pd = ProblemDetails::unauthorized();
    EXPECT_EQ(pd.status, 401);
}

TEST(ProblemDetailsTest, ToJsonContainsFields) {
    auto pd = ProblemDetails::not_found("test");
    auto j = pd.to_json();
    EXPECT_EQ(j["status"].get<int>(), 404);
    EXPECT_EQ(j["title"].get<std::string>(), "Not Found");
    EXPECT_EQ(j["detail"].get<std::string>(), "test");
}
