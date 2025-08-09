include("${CMAKE_CURRENT_SOURCE_DIR}/src/parse_version.cmake")

set(PROJECT "${PREF}_GLSLANG")
set(GLSLANG_CHANGES_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/CHANGES.md")
include(${CMAKE_CURRENT_SOURCE_DIR}/src/parse_version.cmake)
parse_version(${GLSLANG_CHANGES_FILE} GLSLANG)
function(parse_version)
    message(FATAL_ERROR "Overwritten by glslang function [parse_version]")
endfunction()

set(GLSLANG_GENERATED_INCLUDEDIR "${CMAKE_CURRENT_BINARY_DIR}/include")
set(GLSLANG_CHANGES_FILE         "${CMAKE_CURRENT_SOURCE_DIR}/src/CHANGES.md")
set(GLSLANG_BUILD_INFO_H_TMPL    "${CMAKE_CURRENT_SOURCE_DIR}/src/build_info.h.tmpl")
set(GLSLANG_BUILD_INFO_H         "${GLSLANG_GENERATED_INCLUDEDIR}/glslang/build_info.h")

set(major  ${GLSLANG_VERSION_MAJOR})
set(minor  ${GLSLANG_VERSION_MINOR})
set(patch  ${GLSLANG_VERSION_PATCH})
set(flavor ${GLSLANG_VERSION_FLAVOR})
configure_file(${GLSLANG_BUILD_INFO_H_TMPL} ${GLSLANG_BUILD_INFO_H} @ONLY)


list(APPEND BASIC_HEADERS
    glslang_c_interface.h
    glslang_c_shader_types.h
    visibility.h
)
list(TRANSFORM BASIC_HEADERS PREPEND "src/glslang/Include/")

list(APPEND PUBLIC_HEADERS
    resource_limits_c.h
)
list(TRANSFORM PUBLIC_HEADERS PREPEND "src/glslang/Public/")

target_sources(${PREF}glslang.headers INTERFACE
    FILE_SET headers
    TYPE HEADERS
    BASE_DIRS "src/"
    FILES ${PUBLIC_HEADERS} ${PUBLIC_HEADERS}
)

list(APPEND GENERATED_HEADERS
    glslang/build_info.h
)
list(TRANSFORM GENERATED_HEADERS PREPEND "${GLSLANG_GENERATED_INCLUDEDIR}/")

target_sources(${PREF}glslang.headers INTERFACE
    FILE_SET headers
    TYPE HEADERS
    BASE_DIRS ${GLSLANG_GENERATED_INCLUDEDIR}
    FILES ${GENERATED_HEADERS}
)

list(APPEND GLSLANG_SPIRV_SOURCES
    GlslangToSpv.cpp
    InReadableOrder.cpp
    Logger.cpp
    SpvBuilder.cpp
    SpvPostProcess.cpp
    doc.cpp
    SpvTools.cpp
    disassemble.cpp
    CInterface/spirv_c_interface.cpp
)
list(TRANSFORM GLSLANG_SPIRV_SOURCES PREPEND "SPIRV/")

list(APPEND GLSLANG_OSDEPENDENT_SOURCES
    OSDependent/Unix/ossource.cpp
    #OSDependent/Windows/ossource.cpp
)

list(APPEND GLSLANG_GLSLANG_SOURCES
    CInterface/glslang_c_interface.cpp
)

list(APPEND GLSLANG_MACHINEINDEPENDENT_SOURCES
    # MachineIndependent/glslang.y
    MachineIndependent/glslang_tab.cpp
    MachineIndependent/attribute.cpp
    MachineIndependent/Constant.cpp
    MachineIndependent/iomapper.cpp
    MachineIndependent/InfoSink.cpp
    MachineIndependent/Initialize.cpp
    MachineIndependent/IntermTraverse.cpp
    MachineIndependent/Intermediate.cpp
    MachineIndependent/ParseContextBase.cpp
    MachineIndependent/ParseHelper.cpp
    MachineIndependent/PoolAlloc.cpp
    MachineIndependent/RemoveTree.cpp
    MachineIndependent/Scan.cpp
    MachineIndependent/ShaderLang.cpp
    MachineIndependent/SpirvIntrinsics.cpp
    MachineIndependent/SymbolTable.cpp
    MachineIndependent/Versions.cpp
    MachineIndependent/intermOut.cpp
    MachineIndependent/limits.cpp
    MachineIndependent/linkValidate.cpp
    MachineIndependent/parseConst.cpp
    MachineIndependent/reflection.cpp
    MachineIndependent/preprocessor/Pp.cpp
    MachineIndependent/preprocessor/PpAtom.cpp
    MachineIndependent/preprocessor/PpContext.cpp
    MachineIndependent/preprocessor/PpScanner.cpp
    MachineIndependent/preprocessor/PpTokens.cpp
    MachineIndependent/propagateNoContraction.cpp
)

set(ENABLE_HLSL 1)
if(ENABLE_HLSL)
    list(APPEND GLSLANG_MACHINEINDEPENDENT_SOURCES
        HLSL/hlslAttributes.cpp
        HLSL/hlslParseHelper.cpp
        HLSL/hlslScanContext.cpp
        HLSL/hlslOpMap.cpp
        HLSL/hlslTokenStream.cpp
        HLSL/hlslGrammar.cpp
        HLSL/hlslParseables.cpp
    )
endif()


list(APPEND GLSLANG_GENERICCODEGEN_SOURCES
    GenericCodeGen/CodeGen.cpp
    GenericCodeGen/Link.cpp
)

set(GLSLANG_RESOURCELIMITS_SOURCES
    ResourceLimits/ResourceLimits.cpp
    ResourceLimits/resource_limits_c.cpp
)

list(APPEND GLSLANG_DIR_SOURCES
    ${GLSLANG_GLSLANG_SOURCES}
    ${GLSLANG_GENERICCODEGEN_SOURCES}
    ${GLSLANG_OSDEPENDENT_SOURCES}
    ${GLSLANG_MACHINEINDEPENDENT_SOURCES}
    ${GLSLANG_RESOURCELIMITS_SOURCES}
)

list(TRANSFORM GLSLANG_DIR_SOURCES PREPEND "glslang/")

list(APPEND SOURCES
    ${GLSLANG_DIR_SOURCES}
    ${GLSLANG_SPIRV_SOURCES}
)

list(TRANSFORM SOURCES PREPEND "src/")

add_library(${PREF}glslang.object OBJECT ${SOURCES})
add_library(${PREF}glslang.PIC_object OBJECT ${SOURCES})

set_target_properties(${PREF}glslang.PIC_object PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${PREF}glslang.object PRIVATE ${PREF}glslang.headers)
target_link_libraries(${PREF}glslang.PIC_object PRIVATE ${PREF}glslang.headers)

set(PRIVATE_INCLUDES_GENEXPR $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)
target_include_directories(${PREF}glslang.object PRIVATE ${PRIVATE_INCLUDES_GENEXPR})
target_include_directories(${PREF}glslang.PIC_object PRIVATE ${PRIVATE_INCLUDES_GENEXPR})

if(ENABLE_HLSL)
    target_compile_definitions(${PREF}glslang.object PRIVATE ENABLE_HLSL)
    target_compile_definitions(${PREF}glslang.PIC_object PRIVATE ENABLE_HLSL)
endif()
