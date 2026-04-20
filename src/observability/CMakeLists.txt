add_library(cpp_commons_observability STATIC)
add_library(cpp_commons::observability ALIAS cpp_commons_observability)

target_sources(cpp_commons_observability PRIVATE
    json_logger.cpp
    correlation_context.cpp
    health_registry.cpp
)

target_include_directories(cpp_commons_observability PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(cpp_commons_observability PUBLIC
    cpp_commons::kernel
    cpp_commons::errors
    cpp_commons::config
    spdlog::spdlog
    asio::asio
    nlohmann_json::nlohmann_json
)
