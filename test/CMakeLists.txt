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
  precore-objects
  INCLUDES
  ../src
  NO_PROJECT_H
  NO_VERINFO)

add_test(test_main test_main)

add_subdirectory(resources_lmt)

if(ODR_TEST)
  include(odr_test)
  test_odr(PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../include)
  target_link_libraries(test_odr precore-interface)
endif()
