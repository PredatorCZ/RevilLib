project(SDLConvert)

build_target(
  NAME
  sdl_to_xml
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  sdl_to_xml.cpp
  LINKS
  revil-interface
  pugixml-interface
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF SDL to XML converter"
  START_YEAR
  2023)

build_target(
  NAME
  xml_to_sdl
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  xml_to_sdl.cpp
  LINKS
  revil-interface
  pugixml-interface
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF XML to SDL converter"
  START_YEAR
  2023)
