cmake_minimum_required(VERSION 3.3)

project(RevilLib)

if (NOT DEFINED UNICODE)
	set (UNICODE TRUE)
endif()

if (NOT DEFINED RELEASEVER)
	set(RELEASEVER FALSE)
endif()

if (RELEASEVER)
	set(CMAKE_BUILD_TYPE Release)
else()
	set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD 11)

add_library(RevilLib STATIC 
3rd_party/pugixml/src/pugixml.cpp
3rd_party/PreCore/datas/MasterPrinter.cpp
3rd_party/PreCore/datas/reflector.cpp
3rd_party/PreCore/datas/reflectorXML.cpp
src/LMT.cpp
src/LMTBoneTrack.cpp
src/LMTCodecs.cpp
src/LMTEvent.cpp
src/LMTFloatTrack.cpp
src/LMTAnimation.cpp
src/LMTSerialize.cpp
src/LMTXML.cpp
#src/REAsset.cpp
)

include_directories(include/ )
include_directories(src/)
include_directories(3rd_party/PreCore/)
include_directories(3rd_party/pugixml/src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse3")

if (WIN32 AND UNICODE)
	message(STATUS "Compiling with wchar_t")
	add_definitions(-D_UNICODE -DUNICODE)
endif()