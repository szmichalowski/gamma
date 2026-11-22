cmake_minimum_required(VERSION 3.21)
project(gamma LANGUAGES C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

add_executable(gamma)
target_sources(gamma
    PRIVATE
        src/main.c

        src/lexer/lexer.c

        src/parser/parser.c
        src/parser/statement.c
        src/parser/expression.c
        src/parser/type.c

        src/generator/generator.c
        src/generator/elf.c

        src/utils/arena.c
        src/utils/string.c
        src/utils/print.c
)

target_include_directories(gamma PRIVATE api)

target_compile_options(gamma PRIVATE -Wall -Wextra)

add_executable(gamma2)
target_sources(gamma2
    PRIVATE
        src2/main.cpp

        src2/lexer/lexer.cpp

        src2/utils/arena.cpp
        src2/utils/string.cpp
        src2/utils/print.cpp
)

target_include_directories(gamma2 PRIVATE api2)

target_compile_options(gamma2 PRIVATE -Wall -Wextra -Wno-builtin-declaration-mismatch)

include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)

if (supported)
    message(STATUS "IPO/LTO enabled")
    set_property(TARGET gamma PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    set_property(TARGET gamma2 PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "IPO/LTO not supported: <${error}>")
endif()
