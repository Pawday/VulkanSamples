add_library(${PREF}cpptrace INTERFACE)
target_link_libraries(${PREF}cpptrace INTERFACE
    ${PREF}cpptrace.headers
    ${PREF}cpptrace.static
)

list(APPEND STATIC_LIBS
    libdwarf
    cpptrace
)

list(APPEND HEADER_LIBS
    vulkan
    wayland_client
    libdwarf
    cpptrace
)

foreach(S_TARGET ${STATIC_LIBS})
    add_library(${PREF}${S_TARGET}.static STATIC)
endforeach()

foreach(H_TARGET ${HEADER_LIBS})
    add_library(${PREF}${H_TARGET}.headers INTERFACE)
endforeach()

add_subdirectory(Vulkan-Headers)
add_subdirectory(Wayland)
add_subdirectory(libdwarf)
add_subdirectory(cpptrace)

foreach(S_TARGET ${STATIC_LIBS})
    install(TARGETS ${PREF}${S_TARGET}.static DESTINATION "${PREF}${S_TARGET}/lib")
endforeach()

foreach(H_TARGET ${HEADER_LIBS})
    install(TARGETS ${PREF}${H_TARGET}.headers FILE_SET headers DESTINATION "${PREF}${H_TARGET}/include")
endforeach()

