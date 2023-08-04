project(SPACConvert)

build_target(
  NAME
  spac_conv
  TYPE
  ESMODULE
  VERSION
  2
  SOURCES
  spac_conv.cpp
  LINKS
  revil
  INCLUDES
  ../include
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF SPAC Converter"
  START_YEAR
  2019)

# if(UNIX)
#   target_link_libraries(spac_conv libvgmstream)
#   target_compile_definitions(spac_conv PRIVATE USE_VGM)
# endif()
