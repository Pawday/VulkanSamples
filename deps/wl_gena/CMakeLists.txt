if(NOT DEFINED "${PREF}WL_GENA_SOURCE_DIR")
    set(${PREF}WL_GENA_SOURCE_DIR "?" CACHE PATH "Sources location of wl_gena")
endif()
set(SOURCE_DIR ${${PREF}WL_GENA_SOURCE_DIR})

set(PREF_SAVE ${PREF})
set(WL_GENA_PREF "${PREF}dep_")
add_library(${WL_GENA_PREF}libexpat.headers ALIAS ${PREF}expat.headers)
set(PREF ${WL_GENA_PREF})
set(${WL_GENA_PREF}WL_GENA_FIND_PACKAGE_EXPAT OFF)
set(${WL_GENA_PREF}WL_GENA_BUILD_LIBS OFF)
set(${WL_GENA_PREF}WL_GENA_BUILD_EXEC OFF)
add_subdirectory(${SOURCE_DIR} BINDIR)
set(PREF ${PREF_SAVE})

list(APPEND HEADERS
    GenaMain.hh
)
list(TRANSFORM HEADERS PREPEND "${SOURCE_DIR}/include/wl_gena/")

target_sources(${PREF}wl_gena.headers INTERFACE
    FILE_SET headers
    TYPE HEADERS
    BASE_DIRS "${SOURCE_DIR}/include"
    FILES ${HEADERS}
)

add_library(${PREF}wl_gena.object ALIAS ${WL_GENA_PREF}wl_gena.object)
add_library(${PREF}wl_gena.PIC_object ALIAS ${WL_GENA_PREF}wl_gena.PIC_object)
