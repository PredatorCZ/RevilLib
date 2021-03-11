file(GLOB_RECURSE CORE_SOURCE_FILES "*.cpp")

build_target(
  NAME
  RevilLibStatic
  PROPERTIES
  OUTPUT_NAME
  revil
  TYPE
  ${STATIC_TYPE}
  SOURCES
  ${CORE_SOURCE_FILES}
  LINKS
  pugixml
  NO_VERINFO
  NO_PROJECT_H)

target_include_directories(RevilLibStatic PUBLIC ../include)

set_precore_sources(RevilLibStatic master_printer matrix44 reflector
                    reflector_xml no_rtti)

if(USE_PIC AND _is_cool_compiler)
  build_target(
    NAME
    RevilLib
    TYPE
    OBJECT
    SOURCES
    ${CORE_SOURCE_FILES}
    ../3rd_party/pugixml/src/pugixml.cpp
    PROPERTIES
    POSITION_INDEPENDENT_CODE
    ON
    NO_VERINFO
    NO_PROJECT_H)

  target_include_directories(RevilLib PUBLIC ../include
                                             ../3rd_party/pugixml/src)
  set_precore_sources(RevilLib master_printer matrix44 reflector reflector_xml
                      no_rtti)

  if(BUILD_SHARED)
    target_compile_definitions(RevilLib PRIVATE RE_EXPORT)
  endif()
else()
  if(BUILD_SHARED)
    target_compile_definitions(RevilLibStatic PRIVATE RE_EXPORT)
  endif()
  add_library(RevilLib ALIAS RevilLibStatic)
endif()
