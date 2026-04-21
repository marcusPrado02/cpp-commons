add_executable(web_tests
    http_test.cpp
    idempotency_middleware_test.cpp
    content_negotiation_test.cpp
    rate_limit_middleware_test.cpp
)

target_include_directories(web_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/web
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(web_tests PRIVATE
    cpp_commons::web
    cpp_commons::errors
    GTest::gtest_main
)

gtest_discover_tests(web_tests)
