project(DLCExtract)

build_target(
  NAME
  dlc_extract
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  dlc_extract.cpp
  LINKS
  revil-interface
  INCLUDES
  ${CMAKE_SOURCE_DIR}/src/
  AUTHOR
  "Lukas Cone"
  DESCR
  "DLC Archive Extractor"
  START_YEAR
  2024)
