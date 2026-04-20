add_executable(kernel_tests
    result_test.cpp
    option_test.cpp
    identity_test.cpp
    clock_test.cpp
    value_object_test.cpp
    entity_test.cpp
    domain_event_test.cpp
    aggregate_root_test.cpp
    specification_test.cpp
)
target_link_libraries(kernel_tests PRIVATE
    cpp_commons::kernel
    cpp_commons::testing
    GTest::gtest_main
    GTest::gmock
)
gtest_discover_tests(kernel_tests)
