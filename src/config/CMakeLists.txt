add_library(cpp_commons_config STATIC)
add_library(cpp_commons::config ALIAS cpp_commons_config)

target_sources(cpp_commons_config PRIVATE
    config_loader.cpp
    env_source.cpp
    dotenv_source.cpp
)

target_include_directories(cpp_commons_config PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(cpp_commons_config
    PUBLIC
        cpp_commons::kernel
        cpp_commons::errors
    PRIVATE
        nlohmann_json::nlohmann_json
)
