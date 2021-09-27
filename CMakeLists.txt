cmake_minimum_required(VERSION 3.3)

set(CMAKE_CXX_STANDARD 14)
project(RevilToolset VERSION 1.0)

set(CMAKE_INSTALL_RPATH "./")
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
set(TPD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party)
set(BUILD_SHARED_LIBS ON CACHE BOOL "")
set(BUILD_STATIC_OBJECTS OFF CACHE BOOL "")
set(PUGI_SHARED_LIBS ON CACHE BOOL "")
set(PC_SHARED_LIB ON CACHE BOOL "")
add_subdirectory(${TPD_PATH}/RevilLib)

include(targetex)
include(chartype)

add_subdirectory(spac_conv)
