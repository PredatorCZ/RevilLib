cmake_minimum_required(VERSION 3.16)

project(MOD2GLTF VERSION 1.0)

build_target(
  NAME
  mod_to_gltf
  TYPE
  ESMODULE
  SOURCES
  mod_to_gltf
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
  "MTF MOD to GLTF"
  START_YEAR
  2021)

target_precompile_headers(reasset_to_gltf REUSE_FROM gltf_pch)
