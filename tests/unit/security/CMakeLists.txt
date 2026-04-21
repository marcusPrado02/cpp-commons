add_executable(security_tests
    security_test.cpp
    secure_random_test.cpp
    jwt_signer_test.cpp
    jwt_decoder_adversarial_test.cpp
    pii_redactor_adversarial_test.cpp
    key_rotation_service_test.cpp
)

target_include_directories(security_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/security
)

target_link_libraries(security_tests PRIVATE
    cpp_commons::security
    GTest::gtest_main
)

gtest_discover_tests(security_tests)
