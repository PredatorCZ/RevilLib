cmake_minimum_required(VERSION 3.3)

project(RevilToolset VERSION 1.0)

if (CMAKE_SIZEOF_VOID_P EQUAL 4 AND WIN32)
    add_subdirectory(SPACConvert)
endif()
