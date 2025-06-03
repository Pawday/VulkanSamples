macro(declare_deps)

list(APPEND STATIC_LIBS
    libdwarf
    cpptrace
    expat
    wl_gena
)

list(APPEND HEADER_LIBS
    vulkan
    wayland_client
)

list(APPEND HEADER_LIBS
    ${STATIC_LIBS}
)

foreach(H_TARGET ${HEADER_LIBS})
    set(TGT "${PREF}${H_TARGET}.headers")
    add_library(${TGT} INTERFACE)
endforeach()

foreach(S_TARGET ${STATIC_LIBS})
    set(TGT "${PREF}${S_TARGET}.static")
    add_library(${TGT} STATIC)
endforeach()

endmacro()

if(NOT TARGET ${PREF}deps_delclare_once_guard)
    declare_deps()
    add_library(${PREF}deps_delclare_once_guard INTERFACE)
endif()
