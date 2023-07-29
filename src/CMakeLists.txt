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
    ../3rd_party/pvr_core/pvr_decompress.cpp
    LINKS
    pugixml-objects
    precore-objects
    NO_VERINFO
    NO_PROJECT_H)

  target_include_directories(revil-objects PUBLIC ../include ../3rd_party/pvr_core)
endif()

if(BUILD_SHARED_OBJECTS)
  build_target(
    NAME
    revil-shared
    TYPE
    OBJECT
    SOURCES
    ${CORE_SOURCE_FILES}
    ../3rd_party/pvr_core/pvr_decompress.cpp
    PROPERTIES
    POSITION_INDEPENDENT_CODE
    ON
    NO_VERINFO
    NO_PROJECT_H)

  target_include_directories(revil-shared PUBLIC ../include ../3rd_party/pvr_core)

  if(BUILD_SHARED_LIBS)
    target_link_libraries(revil-shared precore pugixml)
  else()
    target_link_libraries(revil-shared precore-objects pugixml-objects)
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

  if(WIN32 OR MINGW)
    set_target_properties(revil PROPERTIES SUFFIX ${PROJECT_VERSION_MAJOR}.dll
                                           PREFIX "")
  else()
    set_target_properties(revil PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
  endif()

  target_compile_definitions(revil INTERFACE RE_IMPORT)
  if(UNIX)
    target_link_libraries(revil -Wl,-z,defs)
  endif()
  install(
    TARGETS revil
    LIBRARY NAMELINK_SKIP DESTINATION lib
    RUNTIME DESTINATION bin)
endif()
