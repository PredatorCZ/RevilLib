cmake_minimum_required(VERSION 3.12)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(FATAL_ERROR "X86 architecture is not supported!")
endif()

project(RevilLib VERSION 1)

# ~~~
# Valid targets:
#  - revil-static: static library target (BUILD_STATIC_LIBS == ON OR BUILD_STATIC_OBJECTS == ON)
#  - revil-objects: static objects target (BUILD_STATIC_OBJECTS == ON)
#  - revil-shared: PIC objects target (BUILD_SHARED_OBJECTS == ON OR BUILD_SHARED_LIBS == ON)
#  - revil: shared library target (BUILD_SHARED_LIBS == ON), revil-shared target will have exported symbols
#
# Definitions:
#  - RE_EXPORT: use to export dynamic symbols from RevilLib target (BUILD_SHARED_LIBS == ON)
#  - RE_IMPORT: use to import dynamic symbols from shared library (API only)
# ~~~

option(ODR_TEST "Enable ODR testing." OFF)
option(BUILD_STATIC_OBJECTS "Builds revil-objects target." ON)
option(BUILD_SHARED_OBJECTS "Builds revil-shared target." OFF)
option(BUILD_STATIC_LIBS "Builds revil-static target." OFF)
option(BUILD_SHARED_LIBS "Builds revil target." OFF)
option(PUGI_AS_SHARED "pugixml target will be shared library instead of being embedded." OFF)

set(CMAKE_CXX_STANDARD 14)
add_subdirectory(3rd_party/PreCore)
include(targetex)
include(chartype)

set(BUILD_SHARED_LIBS_ ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS ${PUGI_AS_SHARED})
add_subdirectory(3rd_party/pugixml)
set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_})

add_subdirectory(src)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  enable_testing()
  add_subdirectory(test)
endif()
