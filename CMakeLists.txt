cmake_minimum_required(VERSION 3.20)

project(Frontier LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED YES)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

add_subdirectory(libodb)
add_subdirectory(objectdb)
