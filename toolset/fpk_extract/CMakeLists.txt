project(FPKExtract)

build_target(
  NAME
  fpk_extract
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  fpk_extract.cpp
  LINKS
  spike-interface
  AUTHOR
  "Lukas Cone"
  DESCR
  "FPK Archive Extractor"
  START_YEAR
  2024)
