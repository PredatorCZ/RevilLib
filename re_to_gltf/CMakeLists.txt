cmake_minimum_required(VERSION 3.16)

project(REAsset2GLTF VERSION 1.0)

build_target(
  NAME
  reasset_to_gltf
  TYPE
  ESMODULE
  SOURCES
  reasset_to_gltf
  LINKS
  revil
  gltf
  AUTHOR
  "Lukas Cone"
  DESCR
  "RE Assets to GLTF"
  START_YEAR
  2021)
