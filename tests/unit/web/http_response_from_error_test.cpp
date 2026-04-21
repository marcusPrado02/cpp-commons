#include <http_response.hpp>
#include <gtest/gtest.h>

using cpp_commons::web::HttpResponse;
using namespace cpp_commons::errors;

TEST(HttpResponseFromErrorTest, NotFoundMaps404) {
    auto r = HttpResponse::from_error(NotFoundError{"item missing"});
    EXPECT_EQ(r.status_code, 404);
}

TEST(HttpResponseFromErrorTest, ValidationErrorMaps422) {
    auto r = HttpResponse::from_error(ValidationError{"bad input"});
    EXPECT_EQ(r.status_code, 422);
}

TEST(HttpResponseFromErrorTest, UnauthorizedErrorMaps401) {
    auto r = HttpResponse::from_error(UnauthorizedError{"no token"});
    EXPECT_EQ(r.status_code, 401);
}

TEST(HttpResponseFromErrorTest, ForbiddenErrorMaps403) {
    auto r = HttpResponse::from_error(ForbiddenError{"not allowed"});
    EXPECT_EQ(r.status_code, 403);
}

TEST(HttpResponseFromErrorTest, ConflictErrorMaps409) {
    auto r = HttpResponse::from_error(ConflictError{"already exists"});
    EXPECT_EQ(r.status_code, 409);
}

TEST(HttpResponseFromErrorTest, RateLimitErrorMaps429) {
    auto r = HttpResponse::from_error(RateLimitError{"slow down"});
    EXPECT_EQ(r.status_code, 429);
}

TEST(HttpResponseFromErrorTest, TimeoutErrorMaps504) {
    auto r = HttpResponse::from_error(TimeoutError{"timed out"});
    EXPECT_EQ(r.status_code, 504);
}

TEST(HttpResponseFromErrorTest, UnknownErrorMaps500) {
    std::runtime_error unknown{"unexpected"};
    auto r = HttpResponse::from_error(unknown);
    EXPECT_EQ(r.status_code, 500);
}

TEST(HttpResponseFromErrorTest, BodyContainsProblemJson) {
    auto r = HttpResponse::from_error(NotFoundError{"missing"});
    EXPECT_NE(r.body.find("Not Found"), std::string::npos);
    EXPECT_NE(r.body.find("404"), std::string::npos);
}
