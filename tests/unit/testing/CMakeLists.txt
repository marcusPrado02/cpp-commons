add_executable(testing_extras_tests
    testing_extras_test.cpp
    fake_implementations_test.cpp
)

target_include_directories(testing_extras_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(testing_extras_tests PRIVATE
    cpp_commons::kernel
    cpp_commons::testing
    GTest::gtest_main
)

gtest_discover_tests(testing_extras_tests)
