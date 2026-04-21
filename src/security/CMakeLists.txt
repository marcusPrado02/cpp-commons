add_library(cpp_commons_security STATIC)
add_library(cpp_commons::security ALIAS cpp_commons_security)

target_sources(cpp_commons_security PRIVATE
    jwt_decoder.cpp
    jwt_signer.cpp
    aes_gcm_provider.cpp
    api_key.cpp
    pii_redactor.cpp
)

target_include_directories(cpp_commons_security PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(cpp_commons_security
    PUBLIC
        cpp_commons::kernel
        cpp_commons::errors
    PRIVATE
        nlohmann_json::nlohmann_json
)

if(OpenSSL_FOUND)
    target_link_libraries(cpp_commons_security PUBLIC OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(cpp_commons_security PUBLIC CPP_COMMONS_HAS_OPENSSL)
endif()
