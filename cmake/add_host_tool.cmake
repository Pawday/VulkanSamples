function(add_host_tool TOOL_NAME TOOL_FOUND)
    unset(${TOOL_NAME}_EXE CACHE)
    find_program(${TOOL_NAME}_EXE ${TOOL_NAME})
    set(${TOOL_NAME}_EXE_FOUND FALSE)
    if(NOT ${${TOOL_NAME}_EXE} STREQUAL "${TOOL_NAME}_EXE-NOTFOUND")
        set(${TOOL_NAME}_EXE_FOUND TRUE)
    endif()

    set(${TOOL_FOUND} ${${TOOL_NAME}_EXE_FOUND} PARENT_SCOPE)

    if(NOT ${${TOOL_NAME}_EXE_FOUND})
        message(STATUS "${TOOL_NAME} not found")
        return()
    endif()

    set(TOOL_TARGET_NAME ${PREF}host_tool_${TOOL_NAME})

    message(STATUS "Found ${TOOL_NAME} ${${TOOL_NAME}_EXE} (imported exec target - ${TOOL_TARGET_NAME})")
    add_executable(${TOOL_TARGET_NAME} IMPORTED GLOBAL)
    set_target_properties(${TOOL_TARGET_NAME} PROPERTIES IMPORTED_LOCATION ${${TOOL_NAME}_EXE})
endfunction()
