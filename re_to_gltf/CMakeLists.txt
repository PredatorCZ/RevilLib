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
  INCLUDES
  ${TPD_PATH}/gltf
  PROPERTIES
  DEFINE_SYMBOL
  GLTF_PCH
  AUTHOR
  "Lukas Cone"
  DESCR
  "RE Assets to GLTF"
  START_YEAR
  2021)

target_precompile_headers(reasset_to_gltf REUSE_FROM gltf_pch)
