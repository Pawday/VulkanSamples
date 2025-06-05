application_sources(${PREF}wl_gena PRIVATE
    Driver.cc
)

application_link_libraries(${PREF}wl_gena
    PRIVATE
        ${PREF}wl_gena.headers
    INTERFACE
        ${PREF}wl_gena.static
        ${PREF}expat.static
)
