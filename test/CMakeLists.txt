cmake_minimum_required (VERSION 3.3)

project(RevilTesting)

set(CMAKE_CXX_STANDARD 11)

build_target(
	TYPE APP
	SOURCES 
        test.cpp
    LINKS
        RevilLib
    INCLUDES
        ../src
        ../include
        ${PRECORE_SOURCE_DIR}
	NO_PROJECT_H
	NO_VERINFO
)

if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -msse3")
	message("Compiler is Clang")
elseif(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -msse3")
	message("Compiler is GNU")
elseif (CMAKE_CXX_COMPILER_ID MATCHES MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W4")
	message("Compiler is MSVC")
else()
	message("Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()