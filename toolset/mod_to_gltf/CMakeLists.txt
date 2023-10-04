project(MOD2GLTF)

build_target(
  NAME
  mod_to_gltf
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  mod_to_gltf.cpp
  LINKS
  revil-interface
  gltf-interface
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF MOD to GLTF"
  START_YEAR
  2021)

project(DWM2GLTF)

build_target(
  NAME
  dwm_to_gltf
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  dwm_to_gltf.cpp
  LINKS
  revil-interface
  gltf-interface
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF MOD to GLTF"
  START_YEAR
  2021)
