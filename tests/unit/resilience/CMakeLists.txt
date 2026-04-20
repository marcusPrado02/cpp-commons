add_executable(resilience_tests
    retry_policy_test.cpp
    circuit_breaker_test.cpp
    bulkhead_test.cpp
)

target_include_directories(resilience_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/resilience
)

target_link_libraries(resilience_tests PRIVATE
    cpp_commons::resilience
    cpp_commons::kernel
    GTest::gtest_main
)

gtest_discover_tests(resilience_tests)
