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
  spike-interface
  AUTHOR
  "Lukas Cone"
  DESCR
  "OOB Archive Extractor"
  START_YEAR
  2022)

install(FILES mhs.files DESTINATION $<IF:$<BOOL:${UNIX}>,data,bin/data>)
