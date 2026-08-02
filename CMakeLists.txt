cmake_minimum_required(VERSION 3.20)
project(maaskk_httpd VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

option(HTTPD_ENABLE_SANITIZERS "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

add_executable(maaskk-httpd
    src/main.c
    src/server.c
)

target_include_directories(maaskk-httpd PRIVATE include)
target_compile_options(maaskk-httpd PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wshadow
)

if(HTTPD_ENABLE_SANITIZERS AND CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(maaskk-httpd PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(maaskk-httpd PRIVATE -fsanitize=address,undefined)
endif()

enable_testing()
add_test(NAME help COMMAND maaskk-httpd --help)

