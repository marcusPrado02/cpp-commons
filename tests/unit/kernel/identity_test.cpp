#include <cpp_commons/kernel/identity.hpp>
#include <gtest/gtest.h>
#include <unordered_map>

using namespace cpp_commons::kernel;

TEST(UUIDTest, GeneratedIsUnique) {
    auto a = UUID::generate();
    auto b = UUID::generate();
    EXPECT_NE(a, b);
}

TEST(UUIDTest, ToStringHasCorrectFormat) {
    auto s = UUID::generate().to_string();
    EXPECT_EQ(s.size(), 36u);
    EXPECT_EQ(s[8],  '-');
    EXPECT_EQ(s[13], '-');
    EXPECT_EQ(s[18], '-');
    EXPECT_EQ(s[23], '-');
}

TEST(StrongIdTest, DefaultConstructedIsUnique) {
    EntityId a, b;
    EXPECT_NE(a, b);
}

TEST(StrongIdTest, EqualityByUUID) {
    auto uuid = UUID::generate();
    EntityId a{uuid}, b{uuid};
    EXPECT_EQ(a, b);
}

TEST(StrongIdTest, ToStringMatchesUUID) {
    auto uuid = UUID::generate();
    EntityId id{uuid};
    EXPECT_EQ(id.to_string(), uuid.to_string());
}

TEST(StrongIdTest, TypesAreDistinct) {
    // EntityId and CorrelationId are different types — won't compile if mixed
    EntityId      eid;
    CorrelationId cid;
    EXPECT_NE(eid.to_string(), cid.to_string());  // different UUIDs
}

// ── std::hash ────────────────────────────────────────────────────────────────

TEST(UUIDHashTest, EqualUUIDsHaveSameHash) {
    auto uuid = UUID::generate();
    EXPECT_EQ(std::hash<UUID>{}(uuid), std::hash<UUID>{}(uuid));
}

TEST(UUIDHashTest, DifferentUUIDsLikelyDifferentHash) {
    auto a = UUID::generate();
    auto b = UUID::generate();
    // Collision possible but astronomically unlikely with UUID v4
    EXPECT_NE(std::hash<UUID>{}(a), std::hash<UUID>{}(b));
}

TEST(UUIDHashTest, UsableInUnorderedMap) {
    std::unordered_map<UUID, int> m;
    auto uuid = UUID::generate();
    m[uuid] = 42;
    EXPECT_EQ(m.at(uuid), 42);
}

TEST(StrongIdHashTest, UsableInUnorderedMap) {
    std::unordered_map<EntityId, std::string> m;
    EntityId id;
    m[id] = "hello";
    EXPECT_EQ(m.at(id), "hello");
}

TEST(StrongIdHashTest, EqualIdsHaveSameHash) {
    auto uuid = UUID::generate();
    EntityId a{uuid}, b{uuid};
    EXPECT_EQ(std::hash<EntityId>{}(a), std::hash<EntityId>{}(b));
}
