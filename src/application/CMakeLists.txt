add_library(cpp_commons_application STATIC)
add_library(cpp_commons::application ALIAS cpp_commons_application)

target_sources(cpp_commons_application PRIVATE
    command_bus.cpp
    query_bus.cpp
    mediator.cpp
    pipeline.cpp
)

target_include_directories(cpp_commons_application PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(cpp_commons_application PUBLIC
    cpp_commons::kernel
    cpp_commons::errors
    cpp_commons::observability
    cpp_commons::resilience
    asio::asio
)
