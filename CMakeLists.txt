cmake_minimum_required(VERSION 3.23)

set(LINK_LIBRARIES_ONLY_TARGETS ON)

project(vkSamples)
set(PREF "${CMAKE_PROJECT_NAME}_")

include(host_tool.cmake)

add_host_tool(glslang HAS_GLSLAND)
if(NOT ${HAS_GLSLAND})
    message(FATAL_ERROR "No glslang")
endif()

add_host_tool(wayland-scanner HAS_WAYLAND_SCANNER)
if(NOT ${HAS_WAYLAND_SCANNER})
    message(FATAL_ERROR "No wayland-scanner")
endif()

function(target_cxx23 tgt)
    set_property(TARGET ${tgt} PROPERTY CXX_STANDARD 23)
    set_property(TARGET ${tgt} PROPERTY CXX_STANDARD_REQUIRED ON)
    set_property(TARGET ${tgt} PROPERTY CXX_EXTENSIONS ON)
endfunction()

function(target_strict_compilation TARGET)
    set(WSHADOW_WARN_OPT "-Wshadow")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(WSHADOW_WARN_OPT "-Wshadow-all")
    endif()

    target_compile_options(${TARGET} PRIVATE
        "-Wall"
        "-Wextra"
        "-pedantic-errors"
        ${WSHADOW_WARN_OPT}
        "-Werror"
    )

    set_property(TARGET ${TARGET} PROPERTY LINK_LIBRARIES_ONLY_TARGETS ON)
endfunction()

function(add_system_library LIBRARY_NAME)
    find_library(${PREF}SYSLIB_${LIBRARY_NAME} ${LIBRARY_NAME} REQUIRED)
    add_library(${PREF}syslib.${LIBRARY_NAME} INTERFACE)
    target_link_libraries(${PREF}syslib.${LIBRARY_NAME} INTERFACE ${${PREF}SYSLIB_${LIBRARY_NAME}})
endfunction()

add_system_library(wayland-client)

add_subdirectory(deps)
add_subdirectory(shaders)

add_executable(${PREF}OffscreenDraw OffscreenDraw.cc Messenger.cc)
target_cxx23(${PREF}OffscreenDraw)
target_strict_compilation(${PREF}OffscreenDraw)
target_link_libraries(${PREF}OffscreenDraw PRIVATE
    ${PREF}vk_headers
    ${PREF}shaders
)
target_compile_definitions(${PREF}OffscreenDraw PRIVATE)


add_library(${PREF}WaylandContext STATIC)
add_subdirectory(wayland)
target_cxx23(${PREF}WaylandContext)
target_strict_compilation(${PREF}WaylandContext)

target_link_libraries(${PREF}WaylandContext PRIVATE ${PREF}syslib.wayland-client)
target_link_libraries(${PREF}WaylandContext PUBLIC
    ${PREF}wayland_client.headers
    ${PREF}vk_headers
)

add_executable(${PREF}WaylandWindow WaylandWindow.cc Messenger.cc)
target_cxx23(${PREF}WaylandWindow)
target_strict_compilation(${PREF}WaylandWindow)

target_link_libraries(${PREF}WaylandWindow PRIVATE ${PREF}WaylandContext)
