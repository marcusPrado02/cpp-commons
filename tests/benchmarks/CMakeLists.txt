add_executable(cpp_commons_benchmarks
    bench_kernel.cpp
    bench_resilience.cpp
    bench_web.cpp
)

target_include_directories(cpp_commons_benchmarks PRIVATE
    ${CMAKE_SOURCE_DIR}/src/observability
    ${CMAKE_SOURCE_DIR}/src/resilience
    ${CMAKE_SOURCE_DIR}/src/web
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(cpp_commons_benchmarks PRIVATE
    cpp_commons::kernel
    cpp_commons::errors
    cpp_commons::observability
    cpp_commons::resilience
    cpp_commons::web
    benchmark::benchmark_main
)
