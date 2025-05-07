set(USE_DWARF 1)
set(USE_LIBUNWIND 1)
set(USE_ADDR2LINE 0)

set(CPPTRACE_SOURCES_PREFIX "src/src")
target_include_directories(${PREF}cpptrace.headers INTERFACE src/include)
target_include_directories(${PREF}cpptrace.static PRIVATE ${CPPTRACE_SOURCES_PREFIX})
target_link_libraries(${PREF}cpptrace.static PRIVATE ${PREF}cpptrace.headers)

list(APPEND CPPTRACE_RELATIVE_SOURCES
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

list(TRANSFORM CPPTRACE_RELATIVE_SOURCES PREPEND "${CPPTRACE_SOURCES_PREFIX}/" OUTPUT_VARIABLE CPPTRACE_SOURCES)
target_sources(${PREF}cpptrace.static PRIVATE ${CPPTRACE_SOURCES})

if(USE_LIBUNWIND)
    list(APPEND UNWIND_BACKEND_DEFINES
        CPPTRACE_UNWIND_WITH_LIBUNWIND
        UNW_LOCAL_ONLY
    )
else()
    list(APPEND UNWIND_BACKEND_DEFINES CPPTRACE_UNWIND_WITH_UNWIND)
endif()

target_compile_definitions(${PREF}cpptrace.static PRIVATE
    ${UNWIND_BACKEND_DEFINES}
    CPPTRACE_DEMANGLE_WITH_CXXABI
    CPPTRACE_GET_SYMBOLS_WITH_NOTHING
)

if(USE_DWARF)
    target_compile_definitions(${PREF}cpptrace.static PRIVATE CPPTRACE_GET_SYMBOLS_WITH_LIBDWARF)
    target_link_libraries(${PREF}cpptrace.static
        PRIVATE
            ${PREF}libdwarf.headers
            ${PREF}libdwarf.static
    )
endif()

if(USE_LIBUNWIND)
    find_library(${PREF}LIB_CPPTRACE_LIBUNWIND_BACKEND unwind REQUIRED)
    target_link_libraries(${PREF}cpptrace.static
        PRIVATE
        ${${PREF}LIB_CPPTRACE_LIBUNWIND_BACKEND}
    )
endif()

if(USE_ADDR2LINE)
    target_compile_definitions(${PREF}cpptrace.static
        PRIVATE
            CPPTRACE_GET_SYMBOLS_WITH_ADDR2LINE
            CPPTRACE_ADDR2LINE_PATH="/usr/bin/addr2line"
    )
endif()

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
