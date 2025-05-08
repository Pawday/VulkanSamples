list(APPEND HEADERS

)

list(TRANSFORM HEADERS PREPEND "src/src/")

target_sources(${PREF}wayland_client.headers INTERFACE
    FILE_SET headers
    TYPE HEADERS
    BASE_DIRS "src/src"
    FILES ${HEADERS}
)
