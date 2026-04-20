add_executable(order_service_example
    main.cpp
)

target_include_directories(order_service_example PRIVATE
    ${CMAKE_SOURCE_DIR}/src/observability
    ${CMAKE_SOURCE_DIR}/src/resilience
    ${CMAKE_SOURCE_DIR}/src/application
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(order_service_example PRIVATE
    cpp_commons::kernel
    cpp_commons::errors
    cpp_commons::observability
    cpp_commons::resilience
    cpp_commons::application
)
