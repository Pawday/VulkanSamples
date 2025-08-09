macro(declare_deps)

list(APPEND BINARY_LIBS
    dwarf
    cpptrace
    expat
    wl_gena
    glslang
)

list(APPEND HEADER_LIBS
    vulkan
    wayland_client
    ${BINARY_LIBS}
)

foreach(H_TARGET ${HEADER_LIBS})
    set(TGT "${PREF}${H_TARGET}.headers")
    add_library(${TGT} INTERFACE)
endforeach()

foreach(BIN_TARGET ${BINARY_LIBS})
    set(TGT "${PREF}${BIN_TARGET}")
    add_library(${TGT}.static STATIC $<TARGET_OBJECTS:${TGT}.object>)
    add_library(${TGT}.shared SHARED $<TARGET_OBJECTS:${TGT}.PIC_object>)
endforeach()

if(${PREF}CPPTRACE_GET_SYMBOLS_WITH_LIBDWARF)
    target_link_libraries(${PREF}cpptrace.static PRIVATE ${PREF}dwarf.static)
    target_link_libraries(${PREF}cpptrace.shared PRIVATE ${PREF}dwarf.shared)
endif()

endmacro()

if(NOT TARGET ${PREF}deps_delclare_once_guard)
    declare_deps()
    add_library(${PREF}deps_delclare_once_guard INTERFACE)
endif()
