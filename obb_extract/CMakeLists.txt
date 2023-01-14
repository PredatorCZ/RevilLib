project(OBBExtract VERSION 1.0)

build_target(
  NAME
  obb_extract
  TYPE
  ESMODULE
  SOURCES
  obb_extract.cpp
  LINKS
  precore
  AUTHOR
  "Lukas Cone"
  DESCR
  "OOB Archive Extractor"
  START_YEAR
  2022)
