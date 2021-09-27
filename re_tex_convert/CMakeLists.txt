cmake_minimum_required(VERSION 3.3)

project(RETEXConvert VERSION 1.0)

build_target(
  NAME
  tex_conv_re
  TYPE
  APP
  SOURCES
  tex_convert.cpp
  LINKS
  precore
  AUTHOR
  "Lukas Cone"
  DESCR
  "RE TEX Converter"
  START_YEAR
  2020)

install(TARGETS tex_conv_re RUNTIME DESTINATION bin)
