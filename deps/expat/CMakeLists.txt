target_compile_definitions(${PREF}expat.static PRIVATE
    XML_GE=0
    XML_CONTEXT_BYTES=0
    HAVE_GETRANDOM=0
)

set(SOURCES_PREFIX "src/expat/lib")

list(APPEND RELATIVE_HEADERS
    expat.h
    expat_external.h
)
list(TRANSFORM RELATIVE_HEADERS PREPEND "${SOURCES_PREFIX}/" OUTPUT_VARIABLE HEADERS)

list(APPEND RELATIVE_SOURCES
    xmlparse.c
    xmlrole.c
    xmltok.c
)
list(TRANSFORM RELATIVE_SOURCES PREPEND "${SOURCES_PREFIX}/" OUTPUT_VARIABLE SOURCES)


set(CODEGEN_INCLUDE "${CMAKE_CURRENT_BINARY_DIR}/include")
target_sources(${PREF}expat.static PRIVATE ${SOURCES})
file(GENERATE OUTPUT "${CODEGEN_INCLUDE}/expat_config.h" CONTENT "// Fake for building")
target_include_directories(${PREF}expat.static PRIVATE ${CODEGEN_INCLUDE})


target_sources(${PREF}expat.headers INTERFACE
    FILE_SET headers
    TYPE HEADERS
    BASE_DIRS "${SOURCES_PREFIX}"
    FILES ${HEADERS}
)
