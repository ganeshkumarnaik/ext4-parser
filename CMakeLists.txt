cmake_minimum_required(VERSION 3.16)

project(ext4-parser
    VERSION 0.1.0
    DESCRIPTION "ext4 filesystem image parser"
    LANGUAGES CXX
)

# Single-config generators otherwise build with no optimisation and no debug info.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(ext4-parser src/main.cpp)

target_include_directories(ext4-parser PRIVATE include)

set_target_properties(ext4-parser PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
)

if(MSVC)
    target_compile_options(ext4-parser PRIVATE /W4 /permissive-)
else()
    target_compile_options(ext4-parser PRIVATE
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
    )
endif()

# Usage: cmake --build build --target build
# CMake reserves the name "clean" and generates that target itself:
#        cmake --build build --target clean
add_custom_target(build DEPENDS ext4-parser)
