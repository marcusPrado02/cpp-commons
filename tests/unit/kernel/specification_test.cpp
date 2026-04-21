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

class NamedSpec : public Specification<Product> {
public:
    explicit NamedSpec(std::string name) : name_{std::move(name)} {}
    [[nodiscard]] bool is_satisfied_by(const Product&) const override { return true; }
    [[nodiscard]] std::string to_string() const override { return name_; }
private:
    std::string name_;
};

static Spec<Product> make_above(int t) {
    return Spec<Product>{std::make_shared<PriceAbove>(t)};
}
static Spec<Product> make_in_stock() {
    return Spec<Product>{std::make_shared<InStock>()};
}
static Spec<Product> named(const std::string& n) {
    return Spec<Product>{std::make_shared<NamedSpec>(n)};
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

// ── Deep composition ──────────────────────────────────────────────────────────

TEST(SpecificationTest, ThreeLevelAndComposition) {
    auto above50  = make_above(50);
    auto above100 = make_above(100);
    auto above200 = make_above(200);
    // price > 50 && price > 100 && price > 200
    auto spec = above50 && above100 && above200;
    EXPECT_TRUE(spec.is_satisfied_by(Product{300, true}));
    EXPECT_FALSE(spec.is_satisfied_by(Product{150, true}));
}

TEST(SpecificationTest, ThreeLevelOrComposition) {
    auto spec = make_above(200) || make_above(100) || make_in_stock();
    EXPECT_TRUE(spec.is_satisfied_by(Product{50, true}));   // in_stock passes
    EXPECT_TRUE(spec.is_satisfied_by(Product{150, false})); // above100 passes
    EXPECT_FALSE(spec.is_satisfied_by(Product{50, false})); // none pass
}

TEST(SpecificationTest, NotOfAndComposition) {
    // !(above100 && in_stock) = price <= 100 || not in_stock
    auto spec = !(make_above(100) && make_in_stock());
    EXPECT_FALSE(spec.is_satisfied_by(Product{200, true}));  // both pass → NOT fails
    EXPECT_TRUE(spec.is_satisfied_by(Product{50, true}));    // price fails → NOT passes
    EXPECT_TRUE(spec.is_satisfied_by(Product{200, false}));  // stock fails → NOT passes
}

TEST(SpecificationTest, AndOfNotComposition) {
    // !above100 && !in_stock = price <= 100 && out of stock
    auto spec = !make_above(100) && !make_in_stock();
    EXPECT_TRUE(spec.is_satisfied_by(Product{50, false}));
    EXPECT_FALSE(spec.is_satisfied_by(Product{50, true}));
    EXPECT_FALSE(spec.is_satisfied_by(Product{200, false}));
}

TEST(SpecificationTest, OrOfNotOrAnd) {
    // (above200 && in_stock) || (!above50)
    auto expensive_and_available = make_above(200) && make_in_stock();
    auto cheap                   = !make_above(50);
    auto spec = expensive_and_available || cheap;
    EXPECT_TRUE(spec.is_satisfied_by(Product{300, true}));  // expensive & in_stock
    EXPECT_TRUE(spec.is_satisfied_by(Product{30, false}));  // cheap
    EXPECT_FALSE(spec.is_satisfied_by(Product{150, true})); // mid-range, in stock
}

// ── to_string ─────────────────────────────────────────────────────────────────

TEST(SpecificationTest, ToStringAndComposition) {
    auto spec = named("price>100") && named("in_stock");
    EXPECT_EQ(spec.to_string(), "(price>100 && in_stock)");
}

TEST(SpecificationTest, ToStringOrComposition) {
    auto spec = named("A") || named("B");
    EXPECT_EQ(spec.to_string(), "(A || B)");
}

TEST(SpecificationTest, ToStringNotComposition) {
    auto spec = !named("in_stock");
    EXPECT_EQ(spec.to_string(), "(!in_stock)");
}

TEST(SpecificationTest, ToStringNestedComposition) {
    auto spec = (named("A") && named("B")) || !named("C");
    EXPECT_EQ(spec.to_string(), "((A && B) || (!C))");
}
