project(DDONSngw)

build_target(
  NAME
  ddon_sngw
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  ddon_sngw.cpp
  LINKS
  spike
  AUTHOR
  "Lukas Cone"
  DESCR
  "DD:Online SNGW converter"
  START_YEAR
  2021)
