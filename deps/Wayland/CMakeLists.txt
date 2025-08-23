if(NOT DEFINED "${PREF}WAYLAND_SOURCE_DIR")
    set(${PREF}WAYLAND_SOURCE_DIR "?" CACHE PATH "Sources location of wayland library")
endif()
set(SOURCE_DIR ${${PREF}WAYLAND_SOURCE_DIR})

list(APPEND HEADERS

)

list(TRANSFORM HEADERS PREPEND "src/src/")

target_sources(${PREF}wayland_client.headers INTERFACE
    FILE_SET headers
    TYPE HEADERS
    BASE_DIRS "${SOURCE_DIR}/src"
    FILES ${HEADERS}
)
