target_sources(${PREF}WaylandContext PRIVATE
    ContextImpl.cc
    RegistryListener.cc
    WindowImpl.cc
    wayland-client-protocol.c
    xdg-shell.c
)

target_include_directories(${PREF}WaylandContext
    PUBLIC
        include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
