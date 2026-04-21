#include <pagination.hpp>
#include <string>
#include <use_case.hpp>

#include <gtest/gtest.h>

using namespace cpp_commons::application;
using namespace cpp_commons::kernel;

struct AddInput {
    int a;
    int b;
};
struct AddOutput {
    int sum;
};
struct AddError {
    std::string msg;
};

struct AddUseCase : UseCase<AddInput, AddOutput, AddError> {
    Result execute(const AddInput& in) override {
        if (in.a < 0 || in.b < 0)
            return Result::err(AddError{"negative inputs"});
        return Result::ok(AddOutput{in.a + in.b});
    }
};

TEST(UseCaseTest, ExecuteReturnsOk) {
    AddUseCase uc;
    auto r = uc.execute({3, 4});
    ASSERT_TRUE(r.is_ok());
    EXPECT_EQ(r.value().sum, 7);
}

TEST(UseCaseTest, ExecuteReturnsErr) {
    AddUseCase uc;
    auto r = uc.execute({-1, 4});
    ASSERT_TRUE(r.is_err());
    EXPECT_EQ(r.error().msg, "negative inputs");
}

TEST(PaginationTest, OffsetCalculation) {
    PageRequest req{.page = 2, .size = 10};
    EXPECT_EQ(req.offset(), 20u);
}

TEST(PaginationTest, HasNext) {
    Page<int> p{.items = {}, .total = 25, .page = 0, .size = 10};
    EXPECT_TRUE(p.has_next());
    Page<int> last{.items = {}, .total = 25, .page = 2, .size = 10};
    EXPECT_FALSE(last.has_next());
}

TEST(PaginationTest, TotalPages) {
    Page<int> p{.items = {}, .total = 21, .page = 0, .size = 10};
    EXPECT_EQ(p.total_pages(), 3u);
}
