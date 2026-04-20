add_library(cpp_commons_resilience STATIC)
add_library(cpp_commons::resilience ALIAS cpp_commons_resilience)

target_sources(cpp_commons_resilience PRIVATE
    retry_policy.cpp
    circuit_breaker.cpp
    bulkhead.cpp
    timeout.cpp
)

target_include_directories(cpp_commons_resilience PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(cpp_commons_resilience PUBLIC
    cpp_commons::kernel
    cpp_commons::errors
    cpp_commons::observability
    asio::asio
)
