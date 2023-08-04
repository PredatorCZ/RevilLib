cmake_minimum_required(VERSION 3.3)

set(CMAKE_CXX_STANDARD 20)
project(RevilToolset)

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
set(NO_OBJECTS
    ON
    CACHE BOOL "" FORCE)

add_subdirectory(${TPD_PATH}/RevilLib)
add_subdirectory(${TPD_PATH}/RevilLib/3rd_party/PreCore/spike
                 ${CMAKE_CURRENT_BINARY_DIR}/spike)
add_subdirectory(${TPD_PATH}/RevilLib/3rd_party/PreCore/gltf)
set_target_properties(spike PROPERTIES OUTPUT_NAME revil_toolset)

include(targetex)
include(version)
toolset_version()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# if (UNIX)
#     if (EXISTS minivgmstream)
#         message("Downloading minivgmstream")
#         file(DOWNLOAD https://github.com/PredatorCZ/minivgmstream/releases/download/<VERSION>/minivgmstream-linux-amd64.tar.xz minivgmstream)
#         file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/vgmstream)
#         execute_process(COMMAND ${CMAKE_COMMAND} -E tar x ../minivgmstream WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/vgmstream)
#     endif()
# 
#     add_library(libvgmstream INTERFACE)
#     target_include_directories(libvgmstream INTERFACE ${CMAKE_BINARY_DIR}/vgmstream/include)
#     target_link_directories(libvgmstream INTERFACE ${CMAKE_BINARY_DIR}/vgmstream/lib)
#     target_link_libraries(libvgmstream INTERFACE vgmstream)
# endif()

add_spike_subdir(spac_conv)
add_spike_subdir(re_tex_convert)
add_spike_subdir(arc_conv)
add_spike_subdir(ddon_sngw)
add_spike_subdir(re_to_gltf)
add_spike_subdir(mod_to_gltf)
add_spike_subdir(mtf_tex_convert)
add_spike_subdir(xfs_conv)
add_spike_subdir(udas_extract)
add_spike_subdir(obb_extract)
# add_spike_subdir(dev)
add_spike_subdir(lmt_to_gltf)
add_spike_subdir(sbk)
add_spike_subdir(stq)
