set(PREF_SAVE ${PREF})
set(WL_GENA_PREF "${PREF}dep_")
add_library(${WL_GENA_PREF}libexpat.headers ALIAS ${PREF}expat.headers)
set(PREF ${WL_GENA_PREF})
set(${WL_GENA_PREF}WL_GENA_FIND_PACKAGE_EXPAT OFF)
set(${WL_GENA_PREF}WL_GENA_LIB_ONLY ON)
add_subdirectory(src EXCLUDE_FROM_ALL)
set(PREF ${PREF_SAVE})

list(APPEND HEADERS
    GenaMain.hh
)
list(TRANSFORM HEADERS PREPEND "src/include/wl_gena/")

target_sources(${PREF}wl_gena.headers INTERFACE
    FILE_SET headers
    TYPE HEADERS
    BASE_DIRS "src/include"
    FILES ${HEADERS}
)

target_sources(${PREF}wl_gena.static PRIVATE $<TARGET_OBJECTS:${WL_GENA_PREF}wl_gena.static>)
target_sources(${PREF}wl_gena.shared PRIVATE $<TARGET_OBJECTS:${WL_GENA_PREF}wl_gena.shared>)
