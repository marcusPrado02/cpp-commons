#include <cpp_commons/kernel/specification.hpp>
#include <gtest/gtest.h>

using cpp_commons::kernel::Spec;
using cpp_commons::kernel::Specification;

struct Product { int price{}; bool in_stock{true}; };

class PriceAbove : public Specification<Product> {
public:
    explicit PriceAbove(int threshold) : threshold_{threshold} {}
    [[nodiscard]] bool is_satisfied_by(const Product& p) const override {
        return p.price > threshold_;
    }
private:
    int threshold_;
};

class InStock : public Specification<Product> {
public:
    [[nodiscard]] bool is_satisfied_by(const Product& p) const override {
        return p.in_stock;
    }
};

static Spec<Product> make_above(int t) {
    return Spec<Product>{std::make_shared<PriceAbove>(t)};
}
static Spec<Product> make_in_stock() {
    return Spec<Product>{std::make_shared<InStock>()};
}

TEST(SpecificationTest, SimpleSpec) {
    auto above100 = make_above(100);
    EXPECT_TRUE(above100.is_satisfied_by(Product{150}));
    EXPECT_FALSE(above100.is_satisfied_by(Product{50}));
}

TEST(SpecificationTest, AndSpec) {
    auto spec = make_above(100) && make_in_stock();
    EXPECT_TRUE(spec.is_satisfied_by(Product{200, true}));
    EXPECT_FALSE(spec.is_satisfied_by(Product{200, false}));
    EXPECT_FALSE(spec.is_satisfied_by(Product{50,  true}));
}

TEST(SpecificationTest, OrSpec) {
    auto spec = make_above(100) || make_in_stock();
    EXPECT_TRUE(spec.is_satisfied_by(Product{200, false}));
    EXPECT_TRUE(spec.is_satisfied_by(Product{50,  true}));
    EXPECT_FALSE(spec.is_satisfied_by(Product{50,  false}));
}

TEST(SpecificationTest, NotSpec) {
    auto spec = !make_in_stock();
    EXPECT_TRUE(spec.is_satisfied_by(Product{0, false}));
    EXPECT_FALSE(spec.is_satisfied_by(Product{0, true}));
}
