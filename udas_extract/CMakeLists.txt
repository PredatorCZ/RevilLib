project(UDASExtract VERSION 1.0)

build_target(
  NAME
  udas_extract
  TYPE
  ESMODULE
  SOURCES
  udas_extract.cpp
  ${TPD_PATH}/mspack/lzxd
  INCLUDES
  ${TPD_PATH}/mspack
  LINKS
  precore
  AUTHOR
  "Lukas Cone"
  DESCR
  "UDAS Archive Extractor"
  START_YEAR
  2022)
