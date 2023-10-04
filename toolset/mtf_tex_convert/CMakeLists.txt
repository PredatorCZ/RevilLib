project(MTFTEXConvert)

build_target(
  NAME
  mtf_tex_to_dds
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  tex_convert.cpp
  LINKS
  revil-interface
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF TEX Converter"
  START_YEAR
  2020)
