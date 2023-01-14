cmake_minimum_required(VERSION 3.16)

project(LMT2GLTF VERSION 1.0)

build_target(
  NAME
  lmt_to_gltf
  TYPE
  ESMODULE
  SOURCES
  lmt_to_gltf
  LINKS
  revil
  gltf
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF LMT to GLTF"
  START_YEAR
  2022)
