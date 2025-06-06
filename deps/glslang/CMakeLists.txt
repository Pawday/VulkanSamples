list(APPEND HEADERS
    glslang_c_interface.h
    glslang_c_shader_types.h
    visibility.h
)

list(TRANSFORM HEADERS PREPEND "src/glslang/Include/")

target_sources(${PREF}glslang.headers INTERFACE
    FILE_SET headers
    TYPE HEADERS
    BASE_DIRS "src/glslang/Include/"
    FILES ${HEADERS}
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

list(APPEND GLSLANG_DIR_SOURCES
    ${GLSLANG_GLSLANG_SOURCES}
    ${GLSLANG_GENERICCODEGEN_SOURCES}
    ${GLSLANG_OSDEPENDENT_SOURCES}
    ${GLSLANG_MACHINEINDEPENDENT_SOURCES}
)

list(TRANSFORM GLSLANG_DIR_SOURCES PREPEND "glslang/")

list(APPEND SOURCES
    ${GLSLANG_DIR_SOURCES}
    ${GLSLANG_SPIRV_SOURCES}
)

list(TRANSFORM SOURCES PREPEND "src/")

target_sources(${PREF}glslang.static PRIVATE ${SOURCES})
target_sources(${PREF}glslang.shared PRIVATE ${SOURCES})

target_link_libraries(${PREF}glslang.static PRIVATE ${PREF}glslang.headers)
target_link_libraries(${PREF}glslang.shared PRIVATE ${PREF}glslang.headers)

set(PRIVATE_INCLUDES_GENEXPR $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)
target_include_directories(${PREF}glslang.static PRIVATE ${PRIVATE_INCLUDES_GENEXPR})
target_include_directories(${PREF}glslang.shared PRIVATE ${PRIVATE_INCLUDES_GENEXPR})

if(ENABLE_HLSL)
    target_compile_definitions(${PREF}glslang.static PRIVATE ENABLE_HLSL)
    target_compile_definitions(${PREF}glslang.shared PRIVATE ENABLE_HLSL)
endif()
