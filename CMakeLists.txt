cmake_minimum_required(VERSION 3.21)
project(gamma LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
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

        src/generator/generator.c
        src/generator/elf.c

        src/arena/arena.c

        src/string/string.c

        src/utils/print.c
)

target_include_directories(gamma PRIVATE api)

target_compile_options(gamma PRIVATE -Wall -Wextra)

include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)

if (supported)
    message(STATUS "IPO/LTO enabled")
    set_property(TARGET gamma PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "IPO/LTO not supported: <${error}>")
endif()
