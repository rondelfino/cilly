cmake_minimum_required(VERSION 3.10)

project(cilly VERSION "0.1" LANGUAGES C)

set(SOURCES src/main.c src/chip8/chip8.c src/chip8/chip8_opcodes.c)

set(TEST_SOURCES
    tests/rom_loading.c
    src/chip8/chip8.c
    src/chip8/chip8_opcodes.c
)

find_package(X11 REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})
add_executable(tests ${TEST_SOURCES})

target_include_directories(cilly PRIVATE include/)
target_include_directories(tests PRIVATE include/)

target_link_libraries(cilly X11::X11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# compiler options
target_compile_options(cilly PRIVATE -Wall -Wextra -g3)
