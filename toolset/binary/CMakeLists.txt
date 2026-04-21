project(MTFBinaryConvert)

build_target(
  NAME
  bin_to_xml
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  bin_to_xml.cpp
  LINKS
  revil-interface
  pugixml-interface
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF binary to XML converter"
  START_YEAR
  2023)

build_target(
  NAME
  xml_to_bin
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  xml_to_bin.cpp
  LINKS
  revil-interface
  pugixml-interface
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF XML to binary converter"
  START_YEAR
  2023)
