cmake_minimum_required(VERSION 3.16)

project(XFSConvert VERSION 1.0)

build_target(
  NAME
  xfs_to_xml
  TYPE
  ESMODULE
  SOURCES
  xfs_to_xml
  LINKS
  revil
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF XFS to XML converter"
  START_YEAR
  2021)
