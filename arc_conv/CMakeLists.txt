cmake_minimum_required(VERSION 3.12)

project(ARCConvert VERSION 1.0)

file(GLOB ZLIB_SOURCES "${TPD_PATH}/zlib/*.c")

build_target(
  NAME
  arc_conv
  TYPE
  APP
  SOURCES
  arc_conv.cpp
  ${TPD_PATH}/mspack/lzxd
  ${ZLIB_SOURCES}
  LINKS
  revil
  INCLUDES
  ${TPD_PATH}/mspack
  ${TPD_PATH}/RevilLib/src/mtf_arc/
  ${TPD_PATH}/zlib
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF Archive Converter"
  START_YEAR
  2020)

install(TARGETS arc_conv RUNTIME DESTINATION bin)
