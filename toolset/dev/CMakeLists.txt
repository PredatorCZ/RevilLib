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
  revil
  INCLUDES
  ${TPD_PATH}/RevilLib/src/mtf_arc/
  AUTHOR
  "Lukas Cone"
  DESCR
  "Validate MTF ARC filesystem"
  START_YEAR
  2022)
