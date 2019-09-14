cmake_minimum_required (VERSION 3.3)

project(LMTCodecsTesting)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 11)

add_executable ("${PROJECT_NAME}" "${PROJECT_NAME}")
include_directories("../3rd_party/PreCore/" "../include/" "../src/")
add_subdirectory(../ "${PROJECT_BINARY_DIR}/RevilLib")
target_link_libraries("${PROJECT_NAME}" RevilLib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse3")