build_target(
  NAME
  test_main
  TYPE
  APP
  SOURCES
  test.cpp
  LINKS
  revil-objects
  pugixml-objects
  INCLUDES
  ../src
  NO_PROJECT_H
  NO_VERINFO)

set_precore_sources(test_main uni)
add_test(test_main test_main)

add_subdirectory(resources_lmt)

if(ODR_TEST)
  include(${PRECORE_SOURCE_DIR}/cmake/odr_test.cmake)
  test_odr(PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../include)
  set_precore_sources(test_odr)
endif()
