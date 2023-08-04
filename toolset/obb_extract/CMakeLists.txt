project(OBBExtract)

build_target(
  NAME
  obb_extract
  TYPE
  ESMODULE
  VERSION
  1
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

install(FILES mhs.files DESTINATION data/mhs.files)
