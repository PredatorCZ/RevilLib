cmake_minimum_required (VERSION 3.3)

project(RevilTesting)

build_target(
	TYPE APP
	SOURCES 
        test.cpp
    LINKS
        RevilLib
    INCLUDES
        ../src
        ../include
	NO_PROJECT_H
	NO_VERINFO
)

set_precore_sources()
