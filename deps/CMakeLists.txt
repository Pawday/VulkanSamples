add_library(${PREF}vk_headers INTERFACE)
add_subdirectory(Vulkan-Headers)

add_library(${PREF}wayland_client.headers INTERFACE)
add_subdirectory(Wayland)

add_library(${PREF}libdwarf.headers INTERFACE)
add_library(${PREF}libdwarf.static STATIC)
add_subdirectory(libdwarf)

add_library(${PREF}cpptrace.headers INTERFACE)
add_library(${PREF}cpptrace.static STATIC)
add_subdirectory(cpptrace)

add_library(${PREF}cpptrace INTERFACE)
target_link_libraries(${PREF}cpptrace INTERFACE
    ${PREF}cpptrace.headers
    ${PREF}cpptrace.static
)

