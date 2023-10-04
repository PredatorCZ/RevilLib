project(RETEXConvert)

build_target(
  NAME
  re_tex_to_dds
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  tex_convert.cpp
  LINKS
  spike-interface
  AUTHOR
  "Lukas Cone"
  DESCR
  "RE TEX Converter"
  START_YEAR
  2020)
