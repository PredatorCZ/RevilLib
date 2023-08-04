project(LMT2GLTF)

build_target(
  NAME
  lmt_to_gltf
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  lmt_to_gltf.cpp
  LINKS
  revil
  gltf
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF LMT to GLTF"
  START_YEAR
  2022)

project(LMT2JSON)

build_target(
  NAME
  lmt_to_json
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  lmt_to_json.cpp
  LINKS
  revil
  gltf
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF LMT to JSON"
  START_YEAR
  2022)
