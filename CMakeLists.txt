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

# ── Dependencies via FetchContent ─────────────────────────────────────────────
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(tl_expected
    GIT_REPOSITORY https://github.com/TartanLlama/expected.git
    GIT_TAG        v1.1.0
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG        asio-1-28-1
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.14.1
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(jwt_cpp
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
    GIT_TAG        v0.7.0
    GIT_SHALLOW    TRUE
)

# Make available (only what we need now; others added per module)
FetchContent_MakeAvailable(tl_expected nlohmann_json)

# Asio: header-only, no CMake install target — wire manually
FetchContent_MakeAvailable(asio)
add_library(asio_interface INTERFACE)
add_library(asio::asio ALIAS asio_interface)
target_include_directories(asio_interface INTERFACE
    ${asio_SOURCE_DIR}/asio/include
)
target_compile_definitions(asio_interface INTERFACE
    ASIO_STANDALONE
    ASIO_NO_DEPRECATED
)

# spdlog
FetchContent_MakeAvailable(spdlog)

# OpenSSL (system)
find_package(OpenSSL QUIET)

# ── INTERFACE (header-only) targets ──────────────────────────────────────────

add_library(cpp_commons_kernel INTERFACE)
add_library(cpp_commons::kernel ALIAS cpp_commons_kernel)
target_include_directories(cpp_commons_kernel INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(cpp_commons_kernel INTERFACE
    tl::expected
    asio::asio
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
    FetchContent_MakeAvailable(googletest)
    enable_testing()
    include(GoogleTest)

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
