target_include_directories(${PREF}WaylandContext PUBLIC include)


if(${${PREF}WAYLAND_DYNAMIC})
    add_subdirectory(dynamic_linking)
else()
    add_subdirectory(direct_linking)
endif()
