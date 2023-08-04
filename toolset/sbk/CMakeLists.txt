project(SBK2JSON)

build_target(
  NAME
  sbk_to_json
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  sbk_to_json.cpp
  LINKS
  precore
  AUTHOR
  "Lukas Cone"
  DESCR
  "Convert MTF Sound Bank to JSON"
  START_YEAR
  2022)
