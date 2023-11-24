project(ARCConvert)

file(GLOB ZLIB_SOURCES "${TPD_PATH}/zlib/*.c")

add_library(zlib-objects OBJECT ${ZLIB_SOURCES})
set_target_properties(zlib-objects PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

build_target(
  NAME
  extract_arc
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  extract_arc.cpp
  ${TPD_PATH}/mspack/lzxd.c
  LINKS
  revil-interface
  zlib-objects
  INCLUDES
  ${TPD_PATH}/mspack
  ${CMAKE_SOURCE_DIR}/src/
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
  VERSION
  1
  SOURCES
  make_arc.cpp
  LINKS
  revil-interface
  zlib-objects
  INCLUDES
  ${CMAKE_SOURCE_DIR}/src/
  ${TPD_PATH}/zlib
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF Archive Maker"
  START_YEAR
  2020)
