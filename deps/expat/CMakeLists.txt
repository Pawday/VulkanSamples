set(SOURCES_PREFIX "src/expat/lib")

list(APPEND HEADERS
    expat.h
    expat_external.h
)
list(TRANSFORM HEADERS PREPEND "${SOURCES_PREFIX}/")

target_sources(${PREF}expat.headers INTERFACE
    FILE_SET headers
    TYPE HEADERS
    BASE_DIRS "${SOURCES_PREFIX}"
    FILES ${HEADERS}
)

list(APPEND SOURCES
    xmlparse.c
    xmlrole.c
    xmltok.c
)
list(TRANSFORM SOURCES PREPEND "${SOURCES_PREFIX}/")

target_sources(${PREF}expat.static PRIVATE ${SOURCES})
target_sources(${PREF}expat.shared PRIVATE ${SOURCES})

set(CODEGEN_INCLUDE "${CMAKE_CURRENT_BINARY_DIR}/include")
file(GENERATE OUTPUT "${CODEGEN_INCLUDE}/expat_config.h" CONTENT "// Fake for building")
target_include_directories(${PREF}expat.static PRIVATE ${CODEGEN_INCLUDE})
target_include_directories(${PREF}expat.shared PRIVATE ${CODEGEN_INCLUDE})

list(APPEND EXPAT_DEFINES
    XML_GE=0
    XML_CONTEXT_BYTES=0
    HAVE_GETRANDOM=0
)

target_compile_definitions(${PREF}expat.static PRIVATE ${EXPAT_DEFINES})
target_compile_definitions(${PREF}expat.shared PRIVATE ${EXPAT_DEFINES})

