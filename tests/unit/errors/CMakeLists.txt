add_executable(errors_tests
    domain_error_test.cpp
    problem_details_test.cpp
    testing_fakes_test.cpp
)
target_link_libraries(errors_tests PRIVATE
    cpp_commons::errors
    cpp_commons::testing
    GTest::gtest_main
)
gtest_discover_tests(errors_tests)
