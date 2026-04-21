#include <string>

#include <cpp_commons/kernel/entity.hpp>

#include <gtest/gtest.h>

using cpp_commons::kernel::Entity;

struct UserId {
    int value{};
    bool operator==(const UserId&) const = default;
};

class User : public Entity<UserId> {
public:
    explicit User(UserId id, std::string name) : Entity{std::move(id)}, name_{std::move(name)} {}
    const std::string& name() const { return name_; }

private:
    std::string name_;
};

TEST(EntityTest, EqualityById) {
    User a{UserId{1}, "Alice"};
    User b{UserId{1}, "Bob"};  // same id, different name
    EXPECT_EQ(a, b);
}

TEST(EntityTest, InequalityById) {
    User a{UserId{1}, "Alice"};
    User b{UserId{2}, "Alice"};  // different id, same name
    EXPECT_NE(a, b);
}

TEST(EntityTest, IdAccessor) {
    User u{UserId{42}, "Carol"};
    EXPECT_EQ(u.id().value, 42);
}
