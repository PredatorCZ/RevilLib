cmake_minimum_required(VERSION 3.3)

project(SPACConvert VERSION 2.0)

set(CMAKE_CURRENT_SOURCE_DIR
    ${TPD_PATH}/vgmstream
    CACHE STRING "" FORCE)

option(USE_FDKAAC "" OFF)
option(BUILD_CLI "" OFF)
option(USE_G7221 "" OFF)

if(WIN32)
  option(BUILD_FB2K "" OFF)
  option(BUILD_WINAMP "" OFF)
  option(BUILD_XMPLAY "" OFF)
else()
  option(BUILD_AUDACIOUS "" OFF)
endif()

option(USE_MPEG "" OFF)
option(USE_VORBIS "" OFF)
option(USE_FFMPEG "" ON)

if(WIN32)
  option(USE_G7221 "" OFF)
  option(USE_G719 "" OFF)
  option(USE_ATRAC9 "" OFF)
  option(USE_CELT "" OFF)
  option(USE_SPEEX "" OFF)
endif()

add_subdirectory(${TPD_PATH}/vgmstream ${CMAKE_BINARY_DIR}/3rd_party/vgmstream)

build_target(
  NAME
  spac_conv
  TYPE
  APP
  SOURCES
  spac_conv.cpp
  LINKS
  libvgmstream
  revil
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF SPAC Converter"
  START_YEAR
  2019)

setup_target(spac_conv TRUE)

install(TARGETS spac_conv RUNTIME DESTINATION bin)
