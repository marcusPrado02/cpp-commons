add_library(cpp_commons_warnings INTERFACE)

target_compile_options(cpp_commons_warnings INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
        -Wall;-Wextra;-Wpedantic;-Werror;
        -Wshadow;-Wnon-virtual-dtor;-Wold-style-cast;
        -Wcast-align;-Wunused;-Woverloaded-virtual;
        -Wconversion;-Wsign-conversion;-Wnull-dereference;
        -Wdouble-promotion;-Wformat=2;-Wimplicit-fallthrough
    >
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4;/WX;/permissive-
    >
)
