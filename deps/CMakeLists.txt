set(USE_EXPORT OFF)

list(APPEND STATIC_LIBS
    libdwarf
    cpptrace
)

list(APPEND HEADER_LIBS
    vulkan
    wayland_client
)

list(APPEND HEADER_LIBS
    ${STATIC_LIBS}
)

if(NOT USE_EXPORT)
    foreach(H_TARGET ${HEADER_LIBS})
        set(TGT "${PREF}${H_TARGET}.headers")
        add_library(${TGT} INTERFACE)
    endforeach()

    foreach(S_TARGET ${STATIC_LIBS})
        set(TGT "${PREF}${S_TARGET}.static")
        add_library(${TGT} STATIC)
    endforeach()

    add_subdirectory(Vulkan-Headers)
    add_subdirectory(Wayland)
    add_subdirectory(libdwarf)
    add_subdirectory(cpptrace)
endif()

list(APPEND ALL_DEPS_TARGETS)

foreach(H_TARGET ${HEADER_LIBS})
    set(TGT "${PREF}${H_TARGET}.headers")
    list(APPEND ALL_DEPS_TARGETS ${TGT})
endforeach()

foreach(S_TARGET ${STATIC_LIBS})
    set(TGT "${PREF}${S_TARGET}.static")
    list(APPEND ALL_DEPS_TARGETS ${TGT})
endforeach()

set(INSTALL_DEPS ON)
if(USE_EXPORT)
    set(INSTALL_DEPS OFF)
endif()

if(INSTALL_DEPS)
    foreach(S_TARGET ${STATIC_LIBS})
        install(TARGETS ${PREF}${S_TARGET}.static EXPORT ${PREF}deps_export DESTINATION "${PREF}${S_TARGET}/lib")
    endforeach()

    foreach(H_TARGET ${HEADER_LIBS})
        install(TARGETS ${PREF}${H_TARGET}.headers EXPORT ${PREF}deps_export FILE_SET headers DESTINATION "${PREF}${H_TARGET}/include")
    endforeach()

    install(EXPORT ${PREF}deps_export DESTINATION "cmake")
endif()

set(EXPORT_NAME ${PREF}deps_export)
if(NOT USE_EXPORT)
    export(EXPORT ${EXPORT_NAME} NAMESPACE exported_)
else()
    include(${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_NAME}.cmake)

    foreach(TARGET ${ALL_DEPS_TARGETS})
        add_library(${TARGET} INTERFACE)
        target_link_libraries(${TARGET} INTERFACE "exported_${TARGET}")
    endforeach()
endif()
