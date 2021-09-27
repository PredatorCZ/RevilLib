cmake_minimum_required(VERSION 3.12)

project(ARCConvert VERSION 1.0)

file(GLOB ZLIB_SOURCES "${TPD_PATH}/zlib/*.c")

add_library(zlib-objects OBJECT ${ZLIB_SOURCES})
set_target_properties(zlib-objects PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

build_target(
  NAME
  extract_arc
  TYPE
  ESMODULE
  SOURCES
  extract_arc.cpp
  ${TPD_PATH}/mspack/lzxd
  LINKS
  revil
  zlib-objects
  INCLUDES
  ${TPD_PATH}/mspack
  ${TPD_PATH}/RevilLib/src/mtf_arc/
  ${TPD_PATH}/zlib
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF Archive Extractor"
  START_YEAR
  2020)

build_target(
  NAME
  make_arc
  TYPE
  ESMODULE
  SOURCES
  make_arc.cpp
  LINKS
  revil
  zlib-objects
  INCLUDES
  ${TPD_PATH}/RevilLib/src/mtf_arc/
  ${TPD_PATH}/zlib
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF Archive Maker"
  START_YEAR
  2020)
