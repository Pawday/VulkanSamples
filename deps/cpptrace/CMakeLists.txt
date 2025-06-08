if(NOT DEFINED ${PREF}CPPTRACE_UNWIND_WITH_LIBUNWIND)
    option("${PREF}CPPTRACE_UNWIND_WITH_LIBUNWIND" OFF)
endif()

set(USE_DWARF 1)


list(APPEND HEADERS
    basic.hpp
    cpptrace.hpp
    exceptions.hpp
    formatting.hpp
    forward.hpp
    from_current.hpp
    gdb_jit.hpp
    io.hpp
    utils.hpp
)
list(TRANSFORM HEADERS PREPEND "src/include/cpptrace/")

target_sources(${PREF}cpptrace.headers INTERFACE
    FILE_SET headers
    TYPE HEADERS
    BASE_DIRS "src/include"
    FILES ${HEADERS}
)

list(APPEND SOURCES
    binary/elf.cpp
    binary/mach-o.cpp
    binary/module_base.cpp
    binary/object.cpp
    binary/pe.cpp
    binary/safe_dl.cpp
    cpptrace.cpp
    ctrace.cpp
    exceptions.cpp
    from_current.cpp
    formatting.cpp
    options.cpp
    utils.cpp
    demangle/demangle_with_cxxabi.cpp
    demangle/demangle_with_nothing.cpp
    demangle/demangle_with_winapi.cpp
    jit/jit_objects.cpp
    snippets/snippet.cpp
    symbols/dwarf/debug_map_resolver.cpp
    symbols/dwarf/dwarf_options.cpp
    symbols/dwarf/dwarf_resolver.cpp
    symbols/symbols_core.cpp
    symbols/symbols_with_addr2line.cpp
    symbols/symbols_with_dbghelp.cpp
    symbols/symbols_with_dl.cpp
    symbols/symbols_with_libbacktrace.cpp
    symbols/symbols_with_libdwarf.cpp
    symbols/symbols_with_nothing.cpp
    unwind/unwind_with_dbghelp.cpp
    unwind/unwind_with_execinfo.cpp
    unwind/unwind_with_libunwind.cpp
    unwind/unwind_with_nothing.cpp
    unwind/unwind_with_unwind.cpp
    unwind/unwind_with_winapi.cpp
    utils/io/file.cpp
    utils/io/memory_file_view.cpp
    utils/error.cpp
    utils/microfmt.cpp
    utils/string_view.cpp
    utils/utils.cpp
    platform/dbghelp_utils.cpp
)
set(CPPTRACE_SOURCES_PREFIX "src/src")
list(TRANSFORM SOURCES PREPEND "${CPPTRACE_SOURCES_PREFIX}/")

target_sources(${PREF}cpptrace.static PRIVATE ${SOURCES})
target_sources(${PREF}cpptrace.shared PRIVATE ${SOURCES})

set(GENEXPR_PRIVATE_INCLUDE_DIR $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${CPPTRACE_SOURCES_PREFIX}>)
target_include_directories(${PREF}cpptrace.static PRIVATE ${GENEXPR_PRIVATE_INCLUDE_DIR})
target_include_directories(${PREF}cpptrace.shared PRIVATE ${GENEXPR_PRIVATE_INCLUDE_DIR})

target_link_libraries(${PREF}cpptrace.static PRIVATE ${PREF}cpptrace.headers)
target_link_libraries(${PREF}cpptrace.shared PRIVATE ${PREF}cpptrace.headers)

list(APPEND DEFINES CPPTRACE_DEMANGLE_WITH_CXXABI)

if(${PREF}CPPTRACE_UNWIND_WITH_LIBUNWIND)
    list(APPEND DEFINES
        CPPTRACE_UNWIND_WITH_LIBUNWIND
        UNW_LOCAL_ONLY
    )
else()
    list(APPEND DEFINES CPPTRACE_UNWIND_WITH_NOTHING)
endif()

if(USE_DWARF)
    list(APPEND DEFINES CPPTRACE_GET_SYMBOLS_WITH_LIBDWARF)
else()
    list(APPEND DEFINES CPPTRACE_GET_SYMBOLS_WITH_NOTHING)
endif()

target_compile_definitions(${PREF}cpptrace.static PRIVATE ${DEFINES})
target_compile_definitions(${PREF}cpptrace.shared PRIVATE ${DEFINES})

if(USE_DWARF)
    target_link_libraries(${PREF}cpptrace.static PRIVATE ${PREF}libdwarf.headers)
    target_link_libraries(${PREF}cpptrace.shared PRIVATE ${PREF}libdwarf.headers)

    target_link_libraries(${PREF}cpptrace.static INTERFACE ${PREF}libdwarf.static)
    target_link_libraries(${PREF}cpptrace.shared INTERFACE ${PREF}libdwarf.shared)
endif()

if(${PREF}CPPTRACE_UNWIND_WITH_LIBUNWIND)
    find_library(${PREF}LIB_CPPTRACE_LIBUNWIND_BACKEND unwind REQUIRED)
    target_link_libraries(${PREF}cpptrace.static PRIVATE ${${PREF}LIB_CPPTRACE_LIBUNWIND_BACKEND})
    target_link_libraries(${PREF}cpptrace.shared PRIVATE ${${PREF}LIB_CPPTRACE_LIBUNWIND_BACKEND})
endif()
