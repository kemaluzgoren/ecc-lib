#
# ecc-lib - Error Correcting Code library
#
# SPDX-License-Identifier: MIT
#
# Usage
# -----
#   cmake -S . -B build
#   cmake --build build
#   ctest --test-dir build -C Debug --output-on-failure
#
# Notes:
#   * Multi-config generators (Visual Studio, Xcode) need -C <Debug|Release>
#     on the ctest line; single-config ones (Ninja, Unix Makefiles) do not and
#     instead take -DCMAKE_BUILD_TYPE=Debug at configure time.
#   * Options (append to the configure line as -D<name>=ON/OFF):
#       ECC_BUILD_TESTS     build the test suite         (default: ON at top level)
#       ECC_BUILD_EXAMPLES  build examples/main.c        (default: OFF)
#   * As a subproject:  add_subdirectory(ecc-lib)  then link ecc::ecc
#     (tests and examples stay off automatically).
#
cmake_minimum_required(VERSION 3.15)

project(ecc-lib
    VERSION 0.1.0
    DESCRIPTION "Generic Hamming codes and channel models"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Only the top-level project drives testing / examples.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(ECC_IS_TOP_LEVEL ON)
else()
    set(ECC_IS_TOP_LEVEL OFF)
endif()

option(ECC_BUILD_TESTS    "Build the ecc test suite" ${ECC_IS_TOP_LEVEL})
option(ECC_BUILD_EXAMPLES "Build the ecc examples"   OFF)

# --- library -----------------------------------------------------------------
add_library(ecc
    src/hamming.c
    src/channel.c
)
add_library(ecc::ecc ALIAS ecc)

target_include_directories(ecc PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ecc PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(ecc PRIVATE /W4)
endif()

# --- tests -----------------------------------------------------------------
if(ECC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# --- examples ------------------------------------------------------------------
if(ECC_BUILD_EXAMPLES)
    add_executable(ecc_example examples/main.c)
    target_link_libraries(ecc_example PRIVATE ecc)
endif()
