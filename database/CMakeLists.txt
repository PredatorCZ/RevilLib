build_target(
    NAME
    make_db
    TYPE
    APP
    SOURCES
    make_db.cpp
    ../src/hash.cpp
    INCLUDES
    ../include
    include
    LINKS
    spike-objects
    pugixml-objects
    AUTHOR
    "Lukas Cone"
    DESCR
    "Create link time revil database"
    START_YEAR
    2023
)

file(GLOB INI_FILES "data/*.ini")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/redb.c
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/make_db
    DEPENDS make_db ${INI_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
    COMMENT "Generating REDB"
)

add_library(generated_redb redb.c)
target_include_directories(generated_redb PUBLIC include)
