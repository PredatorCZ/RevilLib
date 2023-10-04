project(REAsset2GLTF)

build_target(
  NAME
  reasset_to_gltf
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  reasset_to_gltf.cpp
  LINKS
  revil-interface
  gltf-interface
  AUTHOR
  "Lukas Cone"
  DESCR
  "RE Assets to GLTF"
  START_YEAR
  2021)
