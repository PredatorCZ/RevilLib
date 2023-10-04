project(ValidateVFS)

build_target(
  NAME
  validate_arcs
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  validate_arcs.cpp
  LINKS
  revil-interface
  INCLUDES
  ${CMAKE_SOURCE_DIR}/src/mtf_arc/
  AUTHOR
  "Lukas Cone"
  DESCR
  "Validate MTF ARC filesystem"
  START_YEAR
  2022)
