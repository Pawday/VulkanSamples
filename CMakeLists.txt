cmake_minimum_required(VERSION 3.23)

project(vkSamples)
set(PREF "${CMAKE_PROJECT_NAME}_")

option(${PREF}INSTALL_DEPS_ONLY "Only build and install deps as cmake package" OFF)
option(${PREF}USE_PACKAGED_DEPS "Use find_package for deps produced by INSTALL_DEPS_ONLY" OFF)
option(${PREF}INSTALL_HOST_APPS_ONLY "Only build and install host apps as cmake package" OFF)
option(${PREF}USE_PACKAGED_HOST_APPS "Use find_package for host apps produced by INSTALL_HOST_APPS_ONLY" OFF)

option(${PREF}WAYLAND_DYNAMIC "Find wayland-client library at runtime (dlopen)" OFF)

set(DEPS_EXPORT_NAME "${PREF}deps")
if(${PREF}INSTALL_DEPS_ONLY)
    add_subdirectory(deps)
    return()
endif()
if(${PREF}USE_PACKAGED_DEPS)
    find_package(${DEPS_EXPORT_NAME} REQUIRED)
else()
    add_subdirectory(deps EXCLUDE_FROM_ALL)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(add_host_tool)
include(add_system_library)
include(target_cxx23)
include(target_strict_compilation)

function(add_application TARGET_NAME)
    if(NOT TARGET ${PREF}CPPTraceLauncher)
        add_library(${PREF}CPPTraceLauncher STATIC CPPTraceLauncher.cc)
        target_link_libraries(${PREF}CPPTraceLauncher PRIVATE ${PREF}cpptrace.headers ${PREF}cpptrace.static)
    endif()
    if(NOT TARGET ${PREF}Launcher)
        add_library(${PREF}Launcher ALIAS ${PREF}CPPTraceLauncher)
    endif()

    add_library(${TARGET_NAME} STATIC ${ARGN})
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME "${TARGET_NAME}_applib")
    target_cxx23(${TARGET_NAME})
    target_strict_compilation(${TARGET_NAME})

    set(EXEC_TARGET_NAME "${TARGET_NAME}.exec")
    add_executable(${EXEC_TARGET_NAME} $<TARGET_OBJECTS:${PREF}Launcher>)
    set_target_properties(${EXEC_TARGET_NAME} PROPERTIES OUTPUT_NAME "${TARGET_NAME}")
    target_link_libraries(${EXEC_TARGET_NAME} PRIVATE ${PREF}Launcher ${TARGET_NAME})
endfunction()

function(install_application TARGET_NAME)
    set(EXEC_TARGET_NAME "${TARGET_NAME}.exec")
    install(TARGETS ${EXEC_TARGET_NAME} ${ARGN})
endfunction()


list(APPEND HOST_APPLICATIONS
    TestHostApplication
)

foreach(HOST_APPLICATION ${HOST_APPLICATIONS})
    set(TGT "${PREF}host_${HOST_APPLICATION}")
    add_application(${TGT})
endforeach()

set(HOST_APPS_EXPORT_NAME "${PREF}host_apps")
if(${PREF}INSTALL_HOST_APPS_ONLY)
    add_subdirectory(host_apps)
    return()
endif()

if(${PREF}USE_PACKAGED_HOST_APPS)
    find_package(${HOST_APPS_EXPORT_NAME} REQUIRED)
else()
    add_subdirectory(host_apps)
endif()

add_host_tool(glslang HAS_GLSLAND)
if(NOT ${HAS_GLSLAND})
    message(FATAL_ERROR "No glslang")
endif()

add_subdirectory(shaders)

add_executable(${PREF}ICDLoadLeakTest ICDLoadLeakTest.c)

add_library(${PREF}Common STATIC
    Messenger.cc
    SimpleVulkanObjects.cc
)
target_cxx23(${PREF}Common)
target_strict_compilation(${PREF}Common)
target_link_libraries(${PREF}Common PUBLIC
    ${PREF}vulkan.headers
    ${PREF}shaders
)
target_precompile_headers(${PREF}Common
  PUBLIC
    "<vulkan/vk_platform.h>"
    "<vulkan/vulkan.hpp>"
    "<vulkan/vulkan_enums.hpp>"
    "<vulkan/vulkan_handles.hpp>"
    "<vulkan/vulkan_raii.hpp>"
    "<vulkan/vulkan_structs.hpp>"
    "<vulkan/vulkan_to_string.hpp>"
)

add_application(${PREF}OffscreenDraw OffscreenDraw.cc)
target_link_libraries(${PREF}OffscreenDraw PRIVATE ${PREF}Common)

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
target_cxx23(${PREF}WaylandContext)
target_strict_compilation(${PREF}WaylandContext)
target_link_libraries(${PREF}WaylandContext PUBLIC
    ${PREF}vulkan.headers
)

add_application(${PREF}WaylandSurface WaylandSurface.cc)
target_link_libraries(${PREF}WaylandSurface PRIVATE
    ${PREF}Common
    ${PREF}WaylandContext
)

add_application(${PREF}DMABuf DMABuf.cc)
target_link_libraries(${PREF}DMABuf PRIVATE
    ${PREF}Common
    ${PREF}WaylandContext
)

add_application(${PREF}LauncherTest LauncherTest.cc)
target_link_libraries(${PREF}LauncherTest PRIVATE ${PREF}CPPTraceLauncher)
