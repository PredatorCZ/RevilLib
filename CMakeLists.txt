cmake_minimum_required(VERSION 3.3)

set(CMAKE_CXX_STANDARD 17)
project(RevilToolset VERSION 1.0)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

set(TPD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party)
set(BUILD_SHARED_LIBS
    ON
    CACHE BOOL "")
set(BUILD_STATIC_OBJECTS
    OFF
    CACHE BOOL "")

add_subdirectory(${TPD_PATH}/RevilLib/3rd_party/PreCore/spike
                 ${CMAKE_CURRENT_BINARY_DIR}/spike)
add_subdirectory(${TPD_PATH}/RevilLib)
add_subdirectory(${TPD_PATH}/gltf)
set_target_properties(spike PROPERTIES OUTPUT_NAME revil_toolset)

include(targetex)
include(chartype)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_spike_subdir(spac_conv)
add_spike_subdir(re_tex_convert)
add_spike_subdir(arc_conv)
add_spike_subdir(ddon_sngw)
add_spike_subdir(re_to_gltf)
add_spike_subdir(mod_to_gltf)
