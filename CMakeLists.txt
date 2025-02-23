cmake_minimum_required(VERSION 3.23)

set(LINK_LIBRARIES_ONLY_TARGETS ON)

project(vkSamples)
set(PREF "${CMAKE_PROJECT_NAME}_")

include(host_tool.cmake)

add_host_tool(glslang HAS_GLSLAND)
if(NOT ${HAS_GLSLAND})
    message(FATAL_ERROR "No glslang")
endif()

function(target_cxx23 tgt)
    set_property(TARGET ${tgt} PROPERTY CXX_STANDARD 23)
    set_property(TARGET ${tgt} PROPERTY CXX_STANDARD_REQUIRED ON)
    set_property(TARGET ${tgt} PROPERTY CXX_EXTENSIONS ON)
endfunction()

add_subdirectory(deps)
add_subdirectory(shaders)


add_executable(${PREF}OffscreenDraw OffscreenDraw.cc)
target_cxx23(${PREF}OffscreenDraw)
target_link_libraries(${PREF}OffscreenDraw PRIVATE 
    ${PREF}vk_headers
    ${PREF}shaders
)
target_compile_definitions(${PREF}OffscreenDraw PRIVATE)
