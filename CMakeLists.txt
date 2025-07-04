cmake_minimum_required(VERSION 3.23)

project(vkSamples)
set(PREF "${CMAKE_PROJECT_NAME}_")

option(${PREF}INSTALL_DEPS_ONLY "Only build and install deps as cmake package" OFF)
option(${PREF}USE_PACKAGED_DEPS "Use find_package for deps produced by INSTALL_DEPS_ONLY" OFF)
option(${PREF}INSTALL_HOSTTOOLS_ONLY "Only build and install HostBox as cmake package" OFF)
option(${PREF}FIND_HOSTTOOLS "Use find_package for tools produced by INSTALL_HOSTTOOLS_ONLY" OFF)

if(${PREF}INSTALL_DEPS_ONLY AND ${PREF}INSTALL_HOSTTOOLS_ONLY)
    message(FATAL_ERROR "${PREF}INSTALL_DEPS_ONLY and ${PREF}INSTALL_HOSTTOOLS_ONLY cannot be both set")
endif()

option(${PREF}WAYLAND_DYNAMIC "Find wayland-client library at runtime (dlopen)" OFF)

set(DEPS_EXPORT_NAME "${PREF}deps")
if(${PREF}INSTALL_DEPS_ONLY)
    add_subdirectory(deps)
    return()
endif()
if(${PREF}USE_PACKAGED_DEPS)
    find_package(${DEPS_EXPORT_NAME} REQUIRED)
else()
    include(deps/Declare.cmake)
    add_subdirectory(deps EXCLUDE_FROM_ALL)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(add_system_library)
include(target_cxx23)
include(target_strict_compilation)

add_subdirectory(launcher)

function(add_application TARGET_NAME)
    set(LIBRARY_TARGET_NAME "${TARGET_NAME}.applib")
    set(EXEC_TARGET_NAME "${TARGET_NAME}")

    add_library(${LIBRARY_TARGET_NAME} STATIC ${ARGN})
    set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES EXCLUDE_FROM_ALL ON)
    target_link_libraries(${LIBRARY_TARGET_NAME} PRIVATE ${PREF}Launcher.headers)
    set(TARGET_TO_23 ${LIBRARY_TARGET_NAME})
    include(target_cxx23)
    target_strict_compilation(${LIBRARY_TARGET_NAME})

    add_executable(${EXEC_TARGET_NAME} $<TARGET_OBJECTS:${PREF}Launcher>)
    target_link_libraries(${EXEC_TARGET_NAME} PRIVATE ${PREF}Launcher ${LIBRARY_TARGET_NAME})
endfunction()

function(application_sources TARGET_NAME)
    set(LIBRARY_TARGET_NAME "${TARGET_NAME}.applib")
    target_sources(${LIBRARY_TARGET_NAME} ${ARGN})
endfunction()

function(application_link_libraries TARGET_NAME)
    set(LIBRARY_TARGET_NAME "${TARGET_NAME}.applib")
    target_link_libraries(${LIBRARY_TARGET_NAME} ${ARGN})
endfunction()

function(application_install TARGET_NAME)
    install(TARGETS ${TARGET_NAME} ${ARGN})
endfunction()

set(HOSTTOOLS_EXPORT_NAME "${PREF}HostTools")
if(${PREF}FIND_HOSTTOOLS)
    find_package(${HOSTTOOLS_EXPORT_NAME} REQUIRED)
else()
    if(${PREF}INSTALL_HOSTTOOLS_ONLY)
        add_subdirectory(hostbox)
        return()
    endif()
    include(hostbox/Declare.cmake)
    add_subdirectory(hostbox EXCLUDE_FROM_ALL)
endif()

add_subdirectory(shaders)

add_executable(${PREF}ICDLoadLeakTest ICDLoadLeakTest.c)


add_library(${PREF}Common.headers INTERFACE)
target_link_libraries(${PREF}Common.headers
    INTERFACE
        ${PREF}vulkan.headers
        ${PREF}shaders.headers
)

add_library(${PREF}Common STATIC
    Messenger.cc
    SimpleVulkanObjects.cc
)
set(TARGET_TO_23 "${PREF}Common")
include(target_cxx23)
target_strict_compilation(${PREF}Common)
target_link_libraries(${PREF}Common
    PRIVATE
        ${PREF}Common.headers
        ${PREF}shaders
)

add_application(${PREF}OffscreenDraw OffscreenDraw.cc)
application_link_libraries(${PREF}OffscreenDraw PRIVATE ${PREF}Common.headers INTERFACE ${PREF}Common)

if(NOT ${${PREF}WAYLAND_DYNAMIC})
    add_system_library(wayland-client)
endif()
add_library(${PREF}WaylandContext STATIC)
if(NOT ${${PREF}WAYLAND_DYNAMIC})
    target_link_libraries(${PREF}WaylandContext PRIVATE
        ${PREF}syslib.wayland-client
        ${PREF}wayland_client.headers
    )
endif()
add_subdirectory(wayland)
set(TARGET_TO_23 "${PREF}WaylandContext")
include(target_cxx23)
target_strict_compilation(${PREF}WaylandContext)
target_link_libraries(${PREF}WaylandContext PRIVATE
    ${PREF}vulkan.headers
)

add_application(${PREF}WaylandSurface WaylandSurface.cc)
application_link_libraries(${PREF}WaylandSurface
    PRIVATE
        ${PREF}Common.headers
        ${PREF}WaylandContext.headers
    INTERFACE
        ${PREF}WaylandContext
        ${PREF}Common
)

add_application(${PREF}DMABuf DMABuf.cc)
application_link_libraries(${PREF}DMABuf PRIVATE
    PRIVATE
        ${PREF}Common.headers
        ${PREF}WaylandContext.headers
    INTERFACE
        ${PREF}WaylandContext
        ${PREF}Common
)

add_application(${PREF}LauncherTest LauncherTest.cc)
