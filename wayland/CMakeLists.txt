target_sources(${PREF}WaylandWindow PRIVATE
    WaylandWindow.cc
    wayland-client-protocol.c
    xdg-shell.c
)

target_include_directories(${PREF}WaylandWindow PRIVATE .)
