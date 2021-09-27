cmake_minimum_required(VERSION 3.3)

project(RETEXConvert VERSION 1.0)

build_target(
  NAME
  re_tex_to_dds
  TYPE
  ESMODULE
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
