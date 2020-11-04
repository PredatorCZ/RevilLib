build_target(
  NAME
  test_main
  TYPE
  APP
  SOURCES
  test.cpp
  LINKS
  RevilLibStatic
  INCLUDES
  ../src
  ../include
  NO_PROJECT_H
  NO_VERINFO)

set_precore_sources(test_main)
add_test(test_main test_main)
