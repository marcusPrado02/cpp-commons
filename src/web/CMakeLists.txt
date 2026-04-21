add_library(cpp_commons_web STATIC)
add_library(cpp_commons::web ALIAS cpp_commons_web)

target_sources(cpp_commons_web PRIVATE
    http_request.cpp
    http_response.cpp
    middleware_chain.cpp
)

target_include_directories(cpp_commons_web PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(cpp_commons_web
    PUBLIC
        cpp_commons::kernel
        cpp_commons::errors
        cpp_commons::application
    PRIVATE
        nlohmann_json::nlohmann_json
        asio::asio
)

if(ZLIB_FOUND)
    target_compile_definitions(cpp_commons_web PUBLIC CPP_COMMONS_HAS_ZLIB)
    target_link_libraries(cpp_commons_web PRIVATE ZLIB::ZLIB)
endif()
