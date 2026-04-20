add_executable(application_tests
    use_case_test.cpp
    command_bus_test.cpp
    mediator_test.cpp
)

target_include_directories(application_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/application
)

target_link_libraries(application_tests PRIVATE
    cpp_commons::application
    cpp_commons::kernel
    GTest::gtest_main
)

gtest_discover_tests(application_tests)
