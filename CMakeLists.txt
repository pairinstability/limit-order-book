cmake_minimum_required(VERSION 3.22)
project(limit_order_book LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#find_package(absl REQUIRED)

add_library(limitorderbook INTERFACE)
#target_include_directories(limitorderbook INTERFACE
#$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
#)

target_include_directories(limitorderbook INTERFACE
include
)

option(BUILD_MAIN "Build main" ON)
if (BUILD_MAIN)
    add_subdirectory(src)
endif()

option(BUILD_EXAMPLES "Build examples" ON)
if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
