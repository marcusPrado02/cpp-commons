add_executable(config_tests
    settings_test.cpp
    dotenv_source_test.cpp
    yaml_source_test.cpp
    config_watcher_test.cpp
)
target_include_directories(config_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/config
)
target_link_libraries(config_tests PRIVATE
    cpp_commons::config
    cpp_commons::errors
    yaml-cpp::yaml-cpp
    GTest::gtest_main
)
gtest_discover_tests(config_tests)
