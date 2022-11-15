find_package(Python3 REQUIRED COMPONENTS Interpreter)

function(build_classes filename)
    execute_process(COMMAND ${Python3_EXECUTABLE} ${filename}.py WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE VAR)
    file(WRITE ../src/${filename}.inl "${VAR}")
endfunction()

build_classes(mtf_lmt/event)
build_classes(mtf_lmt/float_track)
build_classes(mtf_lmt/bone_track)
build_classes(mtf_lmt/animation)
