cmake_minimum_required(VERSION 3.3)

project(MTFTEXConvert VERSION 1.0)

build_target(
  NAME
  mtf_tex_to_dds
  TYPE
  ESMODULE
  SOURCES
  tex_convert.cpp
  LINKS
  revil
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF TEX Converter"
  START_YEAR
  2020)
