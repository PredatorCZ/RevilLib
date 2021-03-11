cmake_minimum_required(VERSION 3.3)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(FATAL_ERROR "X86 architecture is not supported!")
endif()

project(RevilLib VERSION 1)

# ~~~
# Valid targets:
#  - RevilLibStatic: if BUILD_STATIC then it's target for static library,
#                    otherwise this target will contain objects for static libraries or non-PIE executables
#  - RevilLib: objects for shared libraries or PIE executables
#  - RevilLibShared (only if BUILD_SHARED): target for shared library
#
# NOTE: RevilLibStatic and RevilLib are same for WIN
# NOTE: if BUILD_SHARED then RevilLib will have exported symbols (RE_EXPORT)
#
# Definitions:
#  - RE_EXPORT: use to export dynamic symbols from RevilLib target
#  - RE_IMPORT: use to import dynamic symbols from shared library (API only)
# ~~~

option(USE_PIC "RevilLib target will use PIC." OFF)
option(ODR_TEST "Enable ODR testing." OFF)
option(BUILD_STATIC "Builds RevilLibStatic as a static library." OFF)
option(BUILD_SHARED "Builds RevilLibShared shared library (enables PIC)." OFF)

set(CMAKE_CXX_STANDARD 14)
add_subdirectory(3rd_party/PreCore)
add_subdirectory(3rd_party/pugixml)

include(${PRECORE_SOURCE_DIR}/cmake/targetex.cmake)
include(${PRECORE_SOURCE_DIR}/cmake/chartype.cmake)

set(STATIC_TYPE OBJECT)

if(BUILD_STATIC)
  set(STATIC_TYPE STATIC)
endif()

if(BUILD_SHARED)
  set(USE_PIC ON)
endif()

add_subdirectory(src)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  enable_testing()
  add_subdirectory(test)
endif()

if(BUILD_SHARED)
  build_target(
    NAME
    revil
    TYPE
    SHARED
    LINKS
    RevilLib
    START_YEAR
    2017
    AUTHOR
    "Lukas Cone"
    NO_PROJECT_H)
endif()
