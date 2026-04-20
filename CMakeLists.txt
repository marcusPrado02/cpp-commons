cmake_minimum_required(VERSION 3.25)

project(cpp_commons
    VERSION 0.1.0
    DESCRIPTION "C++ commons library — 12-factor, DDD, CQRS, Hexagonal"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(CPP_COMMONS_BUILD_TESTS      "Build tests"        OFF)
option(CPP_COMMONS_BUILD_EXAMPLES   "Build examples"     OFF)
option(CPP_COMMONS_BUILD_BENCHMARKS "Build benchmarks"   OFF)
option(CPP_COMMONS_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
set(CPP_COMMONS_ENABLE_SANITIZERS "" CACHE STRING
    "Sanitizer to enable: address | thread | memory | (empty)")

# Tooling
include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)
include(cmake/StaticAnalysis.cmake)

# Dependencies via vcpkg
find_package(tl-expected CONFIG REQUIRED)
find_package(asio         CONFIG REQUIRED)
find_package(spdlog       CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL      REQUIRED)
find_package(jwt-cpp      CONFIG REQUIRED)
find_package(absl         CONFIG REQUIRED)

# ── INTERFACE (header-only) targets ──────────────────────────────────────────

add_library(cpp_commons_kernel INTERFACE)
add_library(cpp_commons::kernel ALIAS cpp_commons_kernel)
target_include_directories(cpp_commons_kernel INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(cpp_commons_kernel INTERFACE
    tl-expected::tl-expected
)
target_compile_features(cpp_commons_kernel INTERFACE cxx_std_20)
target_link_libraries(cpp_commons_kernel INTERFACE cpp_commons_warnings)

add_library(cpp_commons_errors INTERFACE)
add_library(cpp_commons::errors ALIAS cpp_commons_errors)
target_include_directories(cpp_commons_errors INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(cpp_commons_errors INTERFACE
    cpp_commons::kernel
    nlohmann_json::nlohmann_json
)

# ── Compiled targets ──────────────────────────────────────────────────────────

add_subdirectory(src/config)
add_subdirectory(src/observability)
add_subdirectory(src/resilience)
add_subdirectory(src/application)
add_subdirectory(src/web)
add_subdirectory(src/security)

# ── Tests ─────────────────────────────────────────────────────────────────────

if(CPP_COMMONS_BUILD_TESTS)
    find_package(GTest CONFIG REQUIRED)
    enable_testing()

    add_library(cpp_commons_testing INTERFACE)
    add_library(cpp_commons::testing ALIAS cpp_commons_testing)
    target_include_directories(cpp_commons_testing INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(cpp_commons_testing INTERFACE
        cpp_commons::kernel
        cpp_commons::errors
        GTest::gtest
        GTest::gmock
    )

    add_subdirectory(tests)
endif()

# ── Examples ──────────────────────────────────────────────────────────────────

if(CPP_COMMONS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ── Install ───────────────────────────────────────────────────────────────────

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS
    cpp_commons_kernel
    cpp_commons_errors
    cpp_commons_config
    cpp_commons_observability
    cpp_commons_resilience
    cpp_commons_application
    cpp_commons_web
    cpp_commons_security
    EXPORT CppCommonsTargets
)

install(EXPORT CppCommonsTargets
    NAMESPACE cpp_commons::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cpp_commons
)

configure_package_config_file(
    cmake/CppCommonsConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/CppCommonsConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cpp_commons
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/CppCommonsConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/CppCommonsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/CppCommonsConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cpp_commons
)
