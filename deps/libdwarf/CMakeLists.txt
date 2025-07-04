set(LIBDWARF_SOURCES_PREFIX "src/src/lib/libdwarf")

target_sources(${PREF}libdwarf.headers INTERFACE
    FILE_SET headers
    TYPE HEADERS
    BASE_DIRS "${LIBDWARF_SOURCES_PREFIX}"
    FILES "${LIBDWARF_SOURCES_PREFIX}/libdwarf.h"
)

set(HAVE_STDINT_H ON)
set(HAVE_UNISTD_H ON)
set(HAVE_FCNTL_H ON)
configure_file("src/cmake/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/include/config.h")

list(APPEND SOURCES
    dwarf_abbrev.c
    dwarf_alloc.c
    dwarf_crc.c
    dwarf_crc32.c
    dwarf_arange.c
    dwarf_debug_sup.c
    dwarf_debugaddr.c
    dwarf_debuglink.c
    dwarf_die_deliv.c
    dwarf_debugnames.c
    dwarf_dsc.c
    dwarf_elf_load_headers.c
    dwarf_elfread.c
    dwarf_elf_rel_detector.c
    dwarf_error.c
    dwarf_fill_in_attr_form.c
    dwarf_find_sigref.c
    dwarf_fission_to_cu.c
    dwarf_form.c
    dwarf_form_class_names.c
    dwarf_frame.c
    dwarf_frame2.c
    dwarf_gdbindex.c
    dwarf_global.c
    dwarf_gnu_index.c
    dwarf_groups.c
    dwarf_harmless.c
    dwarf_generic_init.c
    dwarf_init_finish.c
    dwarf_leb.c
    dwarf_line.c
    dwarf_loc.c
    dwarf_loclists.c
    dwarf_locationop_read.c
    dwarf_local_malloc.c
    dwarf_machoread.c
    dwarf_macro.c
    dwarf_macro5.c
    dwarf_memcpy_swap.c
    dwarf_names.c
    dwarf_object_read_common.c
    dwarf_object_detector.c
    dwarf_peread.c
    dwarf_query.c
    dwarf_ranges.c
    dwarf_rnglists.c
    dwarf_safe_arithmetic.c
    dwarf_safe_strcpy.c
    dwarf_secname_ck.c
    dwarf_seekr.c
    dwarf_setup_sections.c
    dwarf_string.h
    dwarf_string.c
    dwarf_stringsection.c
    dwarf_tied.c
    dwarf_str_offsets.c
    dwarf_tsearchhash.c
    dwarf_util.c
    dwarf_xu_index.c
    dwarf_print_lines.c
)
list(TRANSFORM SOURCES PREPEND "${LIBDWARF_SOURCES_PREFIX}/")

target_sources(${PREF}libdwarf.static PRIVATE ${SOURCES})
target_sources(${PREF}libdwarf.shared PRIVATE ${SOURCES})

set(GENEXPR_PRIVATE_INCLUDE_DIR $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)
target_include_directories(${PREF}libdwarf.static PRIVATE ${GENEXPR_PRIVATE_INCLUDE_DIR})
target_include_directories(${PREF}libdwarf.shared PRIVATE ${GENEXPR_PRIVATE_INCLUDE_DIR})
