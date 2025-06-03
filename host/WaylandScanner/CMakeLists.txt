application_sources(${PREF}WaylandScanner PRIVATE 
    Driver.cc
)

application_link_libraries(${PREF}WaylandScanner
    PRIVATE
        ${PREF}wl_gena.headers
    INTERFACE
        ${PREF}wl_gena.static
        ${PREF}expat.static
)
