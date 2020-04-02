cmake_minimum_required(VERSION 3.3)

project(RevilLib)

if(NOT TARGETEX_LOCATION)
	set(TARGETEX_LOCATION 3rd_party/PreCore/cmake)
	include(${TARGETEX_LOCATION}/targetex.cmake)
	include(${TARGETEX_LOCATION}/chartype.cmake)
endif()

add_subdirectory(3rd_party/precore)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse3")
file(GLOB_RECURSE CORE_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

build_target(
	TYPE STATIC
    SOURCES
		${CORE_SOURCE_FILES}
		3rd_party/pugixml/src/pugixml.cpp
		3rd_party/PreCore/datas/MasterPrinter.cpp
		3rd_party/PreCore/datas/Matrix44.cpp
		3rd_party/PreCore/datas/reflector.cpp
		3rd_party/PreCore/datas/reflectorXML.cpp
    INCLUDES
		src
		include
		3rd_party/PreCore
		3rd_party/pugixml/src
	NO_PROJECT_H
	NO_VERINFO
)

