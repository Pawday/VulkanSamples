target_sources(${PREF}WaylandContext PRIVATE
    WaylandContextImpl.cc
    wayland-client-protocol.c
    xdg-shell.c
)

target_include_directories(${PREF}WaylandContext PRIVATE .)
