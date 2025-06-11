macro(declare_tools)

list(APPEND TOOLS
    glslc
    wl_gena
)


foreach(TOOL ${TOOLS})
    add_application(${PREF}host_${TOOL})
endforeach()


endmacro()

if(NOT TARGET ${PREF}tools_delclare_once_guard)
    declare_tools()
    add_library(${PREF}tools_delclare_once_guard INTERFACE)
endif()
