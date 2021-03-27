file(GLOB_RECURSE CORE_SOURCE_FILES "*.cpp")

if(BUILD_STATIC_LIBS)
  set(BUILD_STATIC_OBJECTS ON)
endif()

if(BUILD_SHARED_LIBS)
  set(BUILD_SHARED_OBJECTS ON)
endif()

if(BUILD_STATIC_OBJECTS OR (MSVC AND BUILD_SHARED_OBJECTS))
  build_target(
    NAME
    revil-objects
    TYPE
    OBJECT
    SOURCES
    ${CORE_SOURCE_FILES}
    LINKS
    pugixml
    NO_VERINFO
    NO_PROJECT_H)

  target_include_directories(revil-objects PUBLIC ../include)

  set_precore_sources(revil-objects master_printer matrix44 reflector
                      reflector_xml no_rtti)
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
  set_precore_sources(revil-shared master_printer matrix44 reflector
                      reflector_xml no_rtti)

  if(PUGI_AS_SHARED)
    target_link_libraries(revil-shared pugixml)
  else()
    target_include_directories(revil-shared PRIVATE ../3rd_party/pugixml/src)
    target_sources(revil-shared PRIVATE ../3rd_party/pugixml/src/pugixml.cpp)
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
    pugixml
    revil-objects
    NO_VERINFO
    NO_PROJECT_H)
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
endif()
