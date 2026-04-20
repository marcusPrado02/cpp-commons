add_executable(observability_tests
    correlation_context_test.cpp
    health_registry_test.cpp
)

target_include_directories(observability_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/observability
)

target_link_libraries(observability_tests PRIVATE
    cpp_commons::observability
    cpp_commons::kernel
    GTest::gtest_main
)

gtest_discover_tests(observability_tests)
