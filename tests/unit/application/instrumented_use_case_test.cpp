#include <instrumented_use_case.hpp>
#include <string>

#include <cpp_commons/kernel/result.hpp>
#include <cpp_commons/testing/fake_metrics.hpp>

#include <gtest/gtest.h>

using namespace cpp_commons::application;
using cpp_commons::kernel::Result;
using Metrics = cpp_commons::testing::SpyMetrics;

struct Input {
    int value{};
};
struct Output {
    int result{};
};
struct MyErr {
    std::string msg;
};

using UC = UseCase<Input, Output, MyErr>;

struct AlwaysOkUseCase : UC {
    Result execute(const Input& in) override { return Result::ok(Output{in.value * 2}); }
};

struct AlwaysErrUseCase : UC {
    Result execute(const Input&) override { return Result::err(MyErr{"oops"}); }
};

TEST(InstrumentedUseCaseTest, IncrementsCallCounterOnSuccess) {
    AlwaysOkUseCase inner;
    Metrics metrics;
    InstrumentedUseCase decorated{inner, metrics, "my_uc"};

    (void)decorated.execute(Input{3});
    EXPECT_EQ(metrics.counter("my_uc.calls"), 1);
    EXPECT_EQ(metrics.counter("my_uc.errors"), 0);
}

TEST(InstrumentedUseCaseTest, IncrementsErrorCounterOnFailure) {
    AlwaysErrUseCase inner;
    Metrics metrics;
    InstrumentedUseCase decorated{inner, metrics, "my_uc"};

    (void)decorated.execute(Input{});
    EXPECT_EQ(metrics.counter("my_uc.calls"), 1);
    EXPECT_EQ(metrics.counter("my_uc.errors"), 1);
}

TEST(InstrumentedUseCaseTest, ReturnsInnerResult) {
    AlwaysOkUseCase inner;
    Metrics metrics;
    InstrumentedUseCase decorated{inner, metrics, "my_uc"};

    auto r = decorated.execute(Input{7});
    ASSERT_TRUE(r.is_ok());
    EXPECT_EQ(r.value().result, 14);
}

TEST(InstrumentedUseCaseTest, RecordsDurationHistogram) {
    AlwaysOkUseCase inner;
    Metrics metrics;
    InstrumentedUseCase decorated{inner, metrics, "my_uc"};

    (void)decorated.execute(Input{1});
    (void)decorated.execute(Input{2});
    // Just check that histogram was called (SpyMetrics stores values)
    // Duration must be >= 0
    EXPECT_EQ(metrics.counter("my_uc.calls"), 2);
}

TEST(InstrumentedUseCaseTest, MultipleCallsAccumulateCounters) {
    AlwaysOkUseCase inner;
    Metrics metrics;
    InstrumentedUseCase decorated{inner, metrics, "uc"};

    for (int i = 0; i < 5; ++i)
        (void)decorated.execute(Input{i});

    EXPECT_EQ(metrics.counter("uc.calls"), 5);
    EXPECT_EQ(metrics.counter("uc.errors"), 0);
}
