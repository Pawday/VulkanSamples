target_sources(${PREF}WaylandContext PRIVATE
    Context.cc
    Window.cc
    VulkanWindow.cc

    ClientCoreLibraryPosix.cc
)

add_library(${PREF}WaylandContext.headers.private INTERFACE)
target_include_directories(${PREF}WaylandContext.headers.private INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PREF}WaylandContext PRIVATE
    ${PREF}WaylandContext.headers.private
)
