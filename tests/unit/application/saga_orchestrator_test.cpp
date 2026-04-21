#include <saga_orchestrator.hpp>
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <stdexcept>

using namespace cpp_commons::application;

TEST(SagaOrchestratorTest, AllStepsSucceed) {
    std::vector<std::string> log;
    SagaOrchestrator saga;
    saga.step("a", [&]{ log.push_back("a"); });
    saga.step("b", [&]{ log.push_back("b"); });
    saga.run();
    EXPECT_EQ(log, (std::vector<std::string>{"a", "b"}));
}

TEST(SagaOrchestratorTest, FailureCompensatesInReverse) {
    std::vector<std::string> log;
    SagaOrchestrator saga;
    saga.step("a", [&]{ log.push_back("do-a"); }, [&]{ log.push_back("undo-a"); });
    saga.step("b", [&]{ log.push_back("do-b"); }, [&]{ log.push_back("undo-b"); });
    saga.step("c", [&]{ throw std::runtime_error{"c failed"}; });

    EXPECT_THROW(saga.run(), SagaError);
    EXPECT_EQ(log, (std::vector<std::string>{"do-a", "do-b", "undo-b", "undo-a"}));
}

TEST(SagaOrchestratorTest, FirstStepFailsNoCompensation) {
    std::vector<std::string> log;
    SagaOrchestrator saga;
    saga.step("a", [&]{ throw std::runtime_error{"a failed"}; },
                   [&]{ log.push_back("undo-a"); });

    EXPECT_THROW(saga.run(), SagaError);
    EXPECT_TRUE(log.empty()); // step never succeeded, no compensation
}

TEST(SagaOrchestratorTest, EmptyStepListSucceeds) {
    SagaOrchestrator saga;
    EXPECT_NO_THROW(saga.run());
}

TEST(SagaOrchestratorTest, CompensationWithoutActionIsOptional) {
    std::vector<std::string> log;
    SagaOrchestrator saga;
    saga.step("a", [&]{ log.push_back("a"); }); // no compensation
    saga.step("b", [&]{ throw std::runtime_error{"b failed"}; });

    EXPECT_THROW(saga.run(), SagaError);
    // 'a' succeeded but has no compensation — log shows only "a"
    EXPECT_EQ(log, (std::vector<std::string>{"a"}));
}

TEST(SagaOrchestratorTest, ErrorMessageContainsFailingStepName) {
    SagaOrchestrator saga;
    saga.step("reserve-inventory", [&]{ throw std::runtime_error{"out of stock"}; });

    try {
        saga.run();
        FAIL() << "expected SagaError";
    } catch (const SagaError& e) {
        EXPECT_NE(std::string{e.what()}.find("reserve-inventory"), std::string::npos);
        EXPECT_NE(std::string{e.what()}.find("out of stock"), std::string::npos);
    }
}
