file(GLOB_RECURSE CORE_SOURCE_FILES "*.cpp")

if(NOT NO_OBJECTS)
  build_target(
    NAME
    revil-objects
    TYPE
    OBJECT
    SOURCES
    ${CORE_SOURCE_FILES}
    ${TPD_PATH}/pvr_core/pvr_decompress.cpp
    PROPERTIES
    POSITION_INDEPENDENT_CODE
    ${OBJECTS_PID}
    LINKS
    pugixml-objects
    spike-objects
    NO_VERINFO
    NO_PROJECT_H)

    target_include_directories(revil-objects PUBLIC ../include ${TPD_PATH}/pvr_core)
endif()

if(BUILD_SHARED_LIBS)
  build_target(
    NAME
    revil
    TYPE
    SHARED
    SOURCES
    ${CORE_SOURCE_FILES}
    ${TPD_PATH}/pvr_core/pvr_decompress.cpp
    LINKS
    spike
    pugixml
    START_YEAR
    2017
    AUTHOR
    "Lukas Cone"
    NO_PROJECT_H)

  target_compile_definitions(revil PRIVATE RE_EXPORT)
  target_include_directories(revil PUBLIC ../include ${TPD_PATH}/pvr_core)

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
