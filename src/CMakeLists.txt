file(GLOB_RECURSE CORE_SOURCE_FILES "*.cpp")

if(BUILD_STATIC_LIBS)
  set(BUILD_STATIC_OBJECTS ON)
endif()

if(BUILD_SHARED_LIBS)
  set(BUILD_SHARED_OBJECTS ON)
endif()

if(BUILD_STATIC_OBJECTS)
  build_target(
    NAME
    revil-objects
    TYPE
    OBJECT
    SOURCES
    ${CORE_SOURCE_FILES}
    LINKS
    pugixml-objects
    NO_VERINFO
    NO_PROJECT_H)

  target_include_directories(revil-objects PUBLIC ../include)

  set_precore_sources(
    revil-objects
    master_printer
    matrix44
    reflector
    reflector_xml
    no_rtti
    uni_format)
endif()

if(BUILD_SHARED_OBJECTS)
  build_target(
    NAME
    revil-shared
    TYPE
    OBJECT
    SOURCES
    ${CORE_SOURCE_FILES}
    PROPERTIES
    POSITION_INDEPENDENT_CODE
    ON
    NO_VERINFO
    NO_PROJECT_H)

  target_include_directories(revil-shared PUBLIC ../include)

  if(BUILD_SHARED_LIBS)
    target_link_libraries(revil-shared precore)
  else()
    set_precore_sources(
      revil-shared
      master_printer
      matrix44
      reflector
      reflector_xml
      no_rtti
      uni_format)
  endif()

  if(BUILD_SHARED_LIBS)
    target_link_libraries(revil-shared pugixml)
  else()
    target_link_libraries(revil-shared pugixml-shared)
  endif()
endif()

if(BUILD_STATIC_LIBS)
  build_target(
    NAME
    revil-static
    PROPERTIES
    OUTPUT_NAME
    revil
    TYPE
    STATIC
    LINKS
    revil-objects
    NO_VERINFO
    NO_PROJECT_H)
  install(TARGETS revil-static LIBRARY DESTINATION lib)
endif()

if(BUILD_SHARED_LIBS)
  target_compile_definitions(revil-shared PRIVATE RE_EXPORT)

  build_target(
    NAME
    revil
    TYPE
    SHARED
    LINKS
    revil-shared
    START_YEAR
    2017
    AUTHOR
    "Lukas Cone"
    NO_PROJECT_H)

  target_compile_definitions(revil INTERFACE RE_IMPORT)
  if(NOT MSVC)
    target_link_libraries(revil -Wl,-z,defs)
  endif()
  install(
    TARGETS revil
    LIBRARY NAMELINK_SKIP DESTINATION lib
    RUNTIME DESTINATION bin)
endif()
