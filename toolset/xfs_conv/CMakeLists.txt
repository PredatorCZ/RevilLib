project(XFSConvert)

build_target(
  NAME
  xfs_to_xml
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  xfs_to_xml.cpp
  LINKS
  revil
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF XFS to XML converter"
  START_YEAR
  2021)
