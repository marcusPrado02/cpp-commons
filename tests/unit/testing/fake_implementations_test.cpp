#include <cpp_commons/testing/fake_logger.hpp>
#include <cpp_commons/testing/fake_metrics.hpp>
#include <cpp_commons/testing/fake_repository.hpp>
#include <cpp_commons/testing/builders.hpp>
#include <cpp_commons/kernel/identity.hpp>
#include <gtest/gtest.h>
#include <string>

namespace ct  = cpp_commons::testing;
namespace ck  = cpp_commons::kernel;

// ── FakeLogger ────────────────────────────────────────────────────────────────

TEST(FakeLoggerTest, CapturesAllLevels) {
    ct::FakeLogger logger;
    logger.trace("t"); logger.debug("d"); logger.info("i");
    logger.warn("w");  logger.error("e");
    EXPECT_EQ(logger.entries().size(), 5u);
    EXPECT_EQ(logger.entries()[0].level, "trace");
    EXPECT_EQ(logger.entries()[4].level, "error");
}

TEST(FakeLoggerTest, HasMessageFindsSubstring) {
    ct::FakeLogger logger;
    logger.info("order-123 created");
    EXPECT_TRUE(logger.has_message("order-123"));
    EXPECT_FALSE(logger.has_message("order-999"));
}

TEST(FakeLoggerTest, ClearResetsEntries) {
    ct::FakeLogger logger;
    logger.info("x");
    logger.clear();
    EXPECT_TRUE(logger.empty());
}

// ── SpyMetrics ────────────────────────────────────────────────────────────────

TEST(SpyMetricsTest, CounterStartsAtZero) {
    ct::SpyMetrics m;
    EXPECT_EQ(m.counter("orders.created"), 0);
}

TEST(SpyMetricsTest, IncrementAccumulates) {
    ct::SpyMetrics m;
    m.increment("orders.created");
    m.increment("orders.created");
    EXPECT_EQ(m.counter("orders.created"), 2);
}

TEST(SpyMetricsTest, GaugeOverwrites) {
    ct::SpyMetrics m;
    m.gauge("queue.depth", 10.0);
    m.gauge("queue.depth", 42.0);
    EXPECT_DOUBLE_EQ(m.gauge_value("queue.depth"), 42.0);
}

TEST(SpyMetricsTest, ClearResetsAll) {
    ct::SpyMetrics m;
    m.increment("x");
    m.clear();
    EXPECT_EQ(m.counter("x"), 0);
}

// ── FakeRepository ────────────────────────────────────────────────────────────

struct Tag {};
using TestId = ck::StrongId<Tag>;

struct Item {
    TestId      id_;
    std::string name;
    [[nodiscard]] const TestId& id() const noexcept { return id_; }
};

TEST(FakeRepositoryTest, SaveAndFindById) {
    ct::FakeRepository<Item, TestId> repo;
    TestId id;
    repo.save(Item{id, "widget"});
    auto found = repo.find_by_id(id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "widget");
}

TEST(FakeRepositoryTest, FindByIdMissingReturnsEmpty) {
    ct::FakeRepository<Item, TestId> repo;
    EXPECT_FALSE(repo.find_by_id(TestId{}).has_value());
}

TEST(FakeRepositoryTest, RemoveDeletesEntity) {
    ct::FakeRepository<Item, TestId> repo;
    TestId id;
    repo.save(Item{id, "widget"});
    repo.remove(id);
    EXPECT_FALSE(repo.find_by_id(id).has_value());
}

TEST(FakeRepositoryTest, SaveOverwritesExisting) {
    ct::FakeRepository<Item, TestId> repo;
    TestId id;
    repo.save(Item{id, "v1"});
    repo.save(Item{id, "v2"});
    EXPECT_EQ(repo.find_by_id(id)->name, "v2");
}

// ── Builder ───────────────────────────────────────────────────────────────────

struct Config {
    int         timeout{30};
    std::string host{"localhost"};
    bool        tls{false};
};

TEST(BuilderTest, AppliesStepsInOrder) {
    auto cfg = ct::Builder<Config>{}
        .with([](Config& c) { c.timeout = 60; })
        .with([](Config& c) { c.host = "prod.example.com"; })
        .with([](Config& c) { c.tls = true; })
        .build();

    EXPECT_EQ(cfg.timeout, 60);
    EXPECT_EQ(cfg.host, "prod.example.com");
    EXPECT_TRUE(cfg.tls);
}

TEST(BuilderTest, NoStepsProducesDefaultObject) {
    auto cfg = ct::Builder<Config>{}.build();
    EXPECT_EQ(cfg.timeout, 30);
    EXPECT_FALSE(cfg.tls);
}
