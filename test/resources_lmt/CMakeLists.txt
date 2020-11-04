
set(ani_num_tracks_)
set(ani_tracks_)
set(lmt_num_anims_)
set(lmt_animations_)

function(make_track a_trackName track_compression_ track_type_ track_bone_type_ track_bone_id_ track_weight_)
    configure_file(track_${a_trackName}.xml.tmpl temp)
    file(READ ${CMAKE_CURRENT_BINARY_DIR}/temp ani_tracks__)
    math(EXPR ani_num_tracks__ "${ani_num_tracks__} + 1")
    set(ani_tracks_ ${ani_tracks__} PARENT_SCOPE)
    set(ani_num_tracks_ ${ani_num_tracks__} PARENT_SCOPE)
endfunction()

function(make_anim a_out_name ani_id_ ani_version_ ani_numframes_ ani_loopframe_ ani_num_evgroups_ ani_num_fltgroups_)
    file(READ eventgroup.xml i_ani_evgroups_)
    set(i_curItem_ 0)
    set(ani_evgroups_ " ")

    while(i_curItem_ LESS ani_num_evgroups_)
        string(APPEND ani_evgroups_ ${i_ani_evgroups_})
        math(EXPR i_curItem_ "${i_curItem_} + 1")
    endwhile()

    configure_file(anim.xml.tmpl ${a_out_name}.mtx)

    if (${a_out_name} STREQUAL none)
        file(READ ${CMAKE_CURRENT_BINARY_DIR}/none.mtx lmt_animations__)
        math(EXPR lmt_num_anims__ "${lmt_num_anims_} + 1")
        set(lmt_animations_ ${lmt_animations__} PARENT_SCOPE)
        set(lmt_num_anims_ ${lmt_num_anims__} PARENT_SCOPE)
    endif()

    set(ani_tracks_ "" PARENT_SCOPE)
    set(ani_num_tracks_ "" PARENT_SCOPE)
endfunction()

function(make_lmt a_out_name lmt_version_ lmt_x64_)
configure_file(main.xml.tmpl ${a_out_name}.xml)
endfunction()

# TrackType_LocalRotation
# TrackType_LocalPosition
# TrackType_LocalScale
# TrackType_AbsoluteRotation
# TrackType_AbsolutePosition

# track_compression_ types:
#
# None
# SingleVector3
# HermiteVector3
# StepRotationQuat3
# SphericalRotation
# LinearVector3
# BiLinearVector3_16bit
# BiLinearVector3_8bit
# LinearRotationQuat4_14bit
# BiLinearRotationQuat4_7bit
# BiLinearRotationQuatXW_14bit
# BiLinearRotationQuatYW_14bit
# BiLinearRotationQuatZW_14bit
# BiLinearRotationQuat4_11bit
# BiLinearRotationQuat4_9bit

make_track(hermite none TrackType_LocalPosition 1 11 111)
make_track(raw SphericalRotation TrackType_LocalRotation 2 22 222)
make_anim(none 0 22 10 11 2 0)
make_track(raw SphericalRotation TrackType_LocalRotation 2 22 222)
make_anim(none 0 22 10 11 2 0)
make_lmt(test 22 true)
