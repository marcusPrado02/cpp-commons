add_executable(integration_tests
    full_stack_test.cpp
)

target_include_directories(integration_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/observability
    ${CMAKE_SOURCE_DIR}/src/resilience
    ${CMAKE_SOURCE_DIR}/src/application
    ${CMAKE_SOURCE_DIR}/src/web
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(integration_tests PRIVATE
    cpp_commons::kernel
    cpp_commons::errors
    cpp_commons::observability
    cpp_commons::resilience
    cpp_commons::application
    cpp_commons::web
    cpp_commons::config
    GTest::gtest_main
)

gtest_discover_tests(integration_tests)
