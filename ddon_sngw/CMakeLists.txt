cmake_minimum_required(VERSION 3.12)

project(DDONSngw VERSION 1.0)

build_target(
  NAME
  ddon_sngw
  TYPE
  APP
  SOURCES
  ddon_sngw.cpp
  LINKS
  precore
  AUTHOR
  "Lukas Cone"
  DESCR
  "DD:Online SNGW converter"
  START_YEAR
  2021)

install(TARGETS ddon_sngw RUNTIME DESTINATION bin)
