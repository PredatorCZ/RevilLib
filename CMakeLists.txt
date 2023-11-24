cmake_minimum_required(VERSION 3.13)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(FATAL_ERROR "X86 architecture is not supported!")
endif()

project(RevilLib VERSION 2)

# ~~~
# Valid targets:
#  - revil-objects: static objects target (NO_OBJECTS == OFF)
#  - revil: shared library target (BUILD_SHARED_LIBS == ON), revil-shared target will have exported symbols
#
# Definitions:
#  - RE_EXPORT: use to export dynamic symbols from RevilLib target (BUILD_SHARED_LIBS == ON)
#  - RE_IMPORT: use to import dynamic symbols from shared library (API only)
# ~~~

option(TOOLSET "Build toolset." ON)
option(ODR_TEST "Enable ODR testing." OFF)

option(OBJECTS_PID "Imply PID for all objects." OFF)

option(CLI "" ${TOOLSET})
option(GLTF "" ${TOOLSET})

if(TOOLSET)
  set(EXPOSE_SYMBOLS spike;gltf;revil;pugixml)
endif()

set(TPD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party)
set(CMAKE_CXX_STANDARD 23)
add_compile_options(-Wall -Wextra)

add_subdirectory(${TPD_PATH}/spike)
include(targetex)

add_subdirectory(database)
add_subdirectory(classgen)
add_subdirectory(src)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR} AND NOT NO_OBJECTS)
  enable_testing()
  add_subdirectory(test)
endif()

if(TOOLSET)
  add_subdirectory(toolset)
endif()
