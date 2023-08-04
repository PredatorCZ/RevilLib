project(UDASExtract)

build_target(
  NAME
  udas_extract
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  udas_extract.cpp
  ${TPD_PATH}/mspack/lzxd.c
  INCLUDES
  ${TPD_PATH}/mspack
  LINKS
  spike
  AUTHOR
  "Lukas Cone"
  DESCR
  "UDAS Archive Extractor"
  START_YEAR
  2022)
