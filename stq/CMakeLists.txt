project(STQ2JSON)

build_target(
  NAME
  stq_to_json
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  stq_to_json.cpp
  LINKS
  precore
  AUTHOR
  "Lukas Cone"
  DESCR
  "Convert MTF Sound Stream Request to JSON"
  START_YEAR
  2022)
