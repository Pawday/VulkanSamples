function(add_glsl_shader TARGET_NAME SHADER_FILE STAGE_TYPE)
    set(INPUT_FILE            "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_FILE}")
    set(GLSLC_OUTPUT_FILE_DIR "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_FILE}.d/")
    file(MAKE_DIRECTORY ${GLSLC_OUTPUT_FILE_DIR})
    set(GLSLC_OUTPUT_FILE     "${GLSLC_OUTPUT_FILE_DIR}/${SHADER_FILE}.spv.hex")


    set(COMPILE_COMMAND "$<TARGET_FILE:${PREF}host_glslc>" "${STAGE_TYPE}" "${INPUT_FILE}" "${GLSLC_OUTPUT_FILE}")

    add_custom_command(
        OUTPUT ${GLSLC_OUTPUT_FILE}
        COMMAND ${COMPILE_COMMAND}
        DEPENDS ${INPUT_FILE} "$<TARGET_FILE:${PREF}host_glslc>"
        COMMENT "glslc ${STAGE_TYPE} ${SHADER_FILE}"
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


add_glsl_shader(${PREF}shaders.headers vertex.glsl vert)
add_glsl_shader(${PREF}shaders.headers fragment.glsl frag)

add_glsl_shader(${PREF}shaders.headers trsq_vertex.glsl vert)
add_glsl_shader(${PREF}shaders.headers trsq_fragment.glsl frag)

target_include_directories(${PREF}shaders.headers INTERFACE include)

add_library(${PREF}shaders STATIC spans.cc)
target_link_libraries(${PREF}shaders PRIVATE ${PREF}shaders.headers)

set(TARGET_TO_23 "${PREF}shaders")
include(target_cxx23)
