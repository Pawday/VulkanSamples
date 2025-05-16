set(EXPORT_NAME ${PREF}deps)
set(EXPORT_VERSION "0.0.0")

list(APPEND STATIC_LIBS
    libdwarf
    cpptrace
    expat
)

list(APPEND HEADER_LIBS
    vulkan
    wayland_client
)

list(APPEND HEADER_LIBS
    ${STATIC_LIBS}
)

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
add_subdirectory(cpptrace)
add_subdirectory(expat)
add_subdirectory(libdwarf)



include(CMakePackageConfigHelpers)
configure_package_config_file(Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_NAME}Config.cmake
    INSTALL_DESTINATION cmake/${EXPORT_NAME}
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_NAME}ConfigVersion.cmake
    VERSION ${EXPORT_VERSION}
    COMPATIBILITY ExactVersion
)

set(GENEXPR_BUILD_TYPE $<CONFIG>)
set(GENEXPR_IS_RELEASE $<STREQUAL:${GENEXPR_BUILD_TYPE},Release>)
set(GENEXPR_BUILD_TYPE_PREFIX "$<IF:${GENEXPR_IS_RELEASE},,/${GENEXPR_BUILD_TYPE}>")

foreach(S_TARGET ${STATIC_LIBS})
    install(TARGETS ${PREF}${S_TARGET}.static EXPORT ${EXPORT_NAME} DESTINATION "${EXPORT_NAME}-${EXPORT_VERSION}/${S_TARGET}/lib${GENEXPR_BUILD_TYPE_PREFIX}")
endforeach()

foreach(H_TARGET ${HEADER_LIBS})
    install(TARGETS ${PREF}${H_TARGET}.headers EXPORT ${EXPORT_NAME} FILE_SET headers DESTINATION "${EXPORT_NAME}-${EXPORT_VERSION}/${H_TARGET}/include")
endforeach()

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_NAME}ConfigVersion.cmake
    DESTINATION
        ${EXPORT_NAME}-${EXPORT_VERSION}/cmake
)

install(EXPORT ${EXPORT_NAME} DESTINATION "${EXPORT_NAME}-${EXPORT_VERSION}/cmake")
