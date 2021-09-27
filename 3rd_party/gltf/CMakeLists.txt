cmake_minimum_required(VERSION 3.16)

project(GLTF VERSION 1.0)

add_library(gltf_pch OBJECT gltf)

target_precompile_headers(gltf_pch PRIVATE gltf.h)
target_include_directories(gltf_pch PUBLIC ${TPD_PATH}/gltf)
target_link_libraries(gltf_pch revil)
set_target_properties(gltf_pch PROPERTIES DEFINE_SYMBOL GLTF_PCH
                                          POSITION_INDEPENDENT_CODE TRUE)
