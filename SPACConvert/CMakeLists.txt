cmake_minimum_required(VERSION 3.3)

project(SPACConvert VERSION 1.0)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
set(TPD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../3rd_party)
set(TARGETEX_LOCATION ${TPD_PATH}/precore/cmake)
include(${TARGETEX_LOCATION}/targetex.cmake)
include(${TARGETEX_LOCATION}/chartype.cmake)

set(CMAKE_CURRENT_SOURCE_DIR ${TPD_PATH}/vgmstream CACHE STRING "" FORCE)

set(CMAKE_MODULE_PATH ${TPD_PATH}/vgmstream/cmake;${CMAKE_MODULE_PATH})
set(USE_FDKAAC OFF CACHE BOOL "Use FDK-AAC/QAAC for support of MP4 AAC" FORCE)
set(BUILD_CLI OFF CACHE BOOL "Build vgmstream CLI" FORCE)

if(WIN32)
    set(BUILD_FB2K OFF CACHE BOOL "Build foobar2000 component" FORCE)
    set(BUILD_WINAMP OFF CACHE BOOL "Build Winamp plugin" FORCE)
    set(BUILD_XMPLAY OFF CACHE BOOL "Build XMPlay plugin" FORCE)
else()
    set(BUILD_AUDACIOUS OFF CACHE BOOL "Build Audacious plugin" FORCE)
endif()

set(USE_MPEG OFF CACHE BOOL "Use libmpg123 for support of MPEG (MP1/MP2/MP3)" FORCE)
set(USE_VORBIS OFF CACHE BOOL "Use libvorbis for support of Vorbis" FORCE)
set(USE_FFMPEG ON CACHE BOOL "Use FFmpeg for support of many codecs" FORCE)

if(WIN32)
	set(USE_G7221 OFF CACHE BOOL "Use libg7221_decode for support of ITU-T G.722.1 annex C" FORCE)
	set(USE_G719 OFF CACHE BOOL "Use libg719_decode for support ITU-T G.719" FORCE)
	set(USE_ATRAC9 OFF CACHE BOOL "Use LibAtrac9 for support of ATRAC9" FORCE)
	set(USE_CELT OFF CACHE BOOL "Use libcelt for support of FSB CELT versions 0.6.1 and 0.11.0" FORCE)
endif()

add_subdirectory(${TPD_PATH}/vgmstream ${CMAKE_BINARY_DIR}/3rd_party/vgmstream)

build_target(
	TYPE APP
	SOURCES
        SPACConvert.cpp
        ${TPD_PATH}/precore/datas/MasterPrinter.cpp
		${TPD_PATH}/precore/datas/reflector.cpp
		${TPD_PATH}/precore/datas/reflectorXML.cpp
		${TPD_PATH}/RevilLib/3rd_party/pugixml/src/pugixml.cpp
	INCLUDES
		${TPD_PATH}/precore
		${TPD_PATH}/RevilLib/3rd_party/pugixml/src
	LINKS
		libvgmstream
	AUTHOR "Lukas Cone"
	DESCR "MTF SPAC Converter"
	NAME "SPACConvert"
	START_YEAR 2019
)

setup_target(SPACConvert TRUE)
install_dlls(${CMAKE_INSTALL_PREFIX}/bin)

install(TARGETS SPACConvert
	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
