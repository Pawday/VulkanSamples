add_library(${PREF}WaylandContext.headers INTERFACE)
target_include_directories(${PREF}WaylandContext.headers INTERFACE include)
target_link_libraries(${PREF}WaylandContext PRIVATE ${PREF}WaylandContext.headers)


if(${${PREF}WAYLAND_DYNAMIC})
    add_subdirectory(dynamic_linking)
else()
    add_subdirectory(direct_linking)
endif()
