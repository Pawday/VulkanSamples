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

add_executable(${PREF}ICDLoadLeakTest ICDLoadLeakTest.c)


add_library(${PREF}CPPTraceLauncher STATIC CPPTraceLauncher.cc)
target_link_libraries(${PREF}CPPTraceLauncher PUBLIC ${PREF}cpptrace.headers)

add_library(${PREF}Launcher ALIAS ${PREF}CPPTraceLauncher)

function(add_application TARGET_NAME)
    add_library(${TARGET_NAME} STATIC ${ARGN})
    set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME "${TARGET_NAME}_applib")
    target_cxx23(${TARGET_NAME})
    target_strict_compilation(${TARGET_NAME})

    set(EXEC_TARGET_NAME "${TARGET_NAME}.exec")
    add_executable(${EXEC_TARGET_NAME} $<TARGET_OBJECTS:${PREF}Launcher>)
    set_target_properties(${EXEC_TARGET_NAME} PROPERTIES OUTPUT_NAME "${TARGET_NAME}")
    target_link_libraries(${EXEC_TARGET_NAME} PRIVATE ${PREF}cpptrace.static ${PREF}Launcher ${TARGET_NAME})
endfunction()

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

add_library(${PREF}WaylandContext STATIC)
add_subdirectory(wayland)
target_cxx23(${PREF}WaylandContext)
target_strict_compilation(${PREF}WaylandContext)
target_link_libraries(${PREF}WaylandContext PRIVATE ${PREF}syslib.wayland-client)
target_link_libraries(${PREF}WaylandContext PUBLIC
    ${PREF}wayland_client.headers
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

add_application(${PREF}LauncherTest TestApplication.cc)
target_link_libraries(${PREF}LauncherTest PRIVATE ${PREF}CPPTraceLauncher)
