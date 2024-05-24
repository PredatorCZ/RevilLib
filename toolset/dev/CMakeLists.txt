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
  ${CMAKE_SOURCE_DIR}/src/
  AUTHOR
  "Lukas Cone"
  DESCR
  "Validate MTF ARC filesystem"
  START_YEAR
  2022)

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

project(LMTBones)

build_target(
  NAME
  lmt_bones
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  lmt_bones.cpp
  LINKS
  revil-interface
  AUTHOR
  "Lukas Cone"
  DESCR
  "Collect LMT bone info"
  START_YEAR
  2024)

add_spike_subdir(sbk)
add_spike_subdir(stq)
