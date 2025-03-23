function(add_glsl_shader TARGET_NAME SHADER_FILE STAGE_TYPE)
    set(INPUT_FILE            "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_FILE}")
    set(GLSLC_OUTPUT_FILE_DIR "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_FILE}.d/")
    file(MAKE_DIRECTORY ${GLSLC_OUTPUT_FILE_DIR})
    set(GLSLC_OUTPUT_FILE     "${GLSLC_OUTPUT_FILE_DIR}/${SHADER_FILE}.spv.hex")


    set(COMPILE_COMMAND_DEBUG_FLAG "")
    string(TOUPPER "${CMAKE_BUILD_TYPE}" BT_UPPER)
    if ("${BT_UPPER}" STREQUAL "DEBUG")
        set(COMPILE_COMMAND_DEBUG_FLAG "-gVS")
    endif()

    set(COMPILE_COMMAND "$<TARGET_FILE:${PREF}host_tool_glslang>" "${COMPILE_COMMAND_DEBUG_FLAG}" "-V100" "--quiet" "-S" "${STAGE_TYPE}" "${INPUT_FILE}" "-x" "-o" "${GLSLC_OUTPUT_FILE}")

    add_custom_command(
        OUTPUT ${GLSLC_OUTPUT_FILE}
        COMMAND ${COMPILE_COMMAND}
        DEPENDS ${INPUT_FILE}
        COMMENT "glslang ${STAGE_TYPE} ${SHADER_FILE}"
    )

    if (NOT TARGET ${TARGET_NAME}_gen)
        add_custom_target(${TARGET_NAME}_gen)
    endif()

    target_sources(${TARGET_NAME}_gen PRIVATE ${GLSLC_OUTPUT_FILE})

    if (NOT TARGET ${TARGET_NAME})
        add_library(${TARGET_NAME} INTERFACE)
    endif()
    add_dependencies(${TARGET_NAME} ${TARGET_NAME}_gen)
    target_include_directories(${TARGET_NAME} INTERFACE ${GLSLC_OUTPUT_FILE_DIR})

endfunction()


add_glsl_shader(${PREF}shaders vertex.glsl vert)
add_glsl_shader(${PREF}shaders fragment.glsl frag)

add_glsl_shader(${PREF}shaders trsq_vertex.glsl vert)
add_glsl_shader(${PREF}shaders trsq_fragment.glsl frag)
