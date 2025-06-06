set(EXPORT_VERSION "0.0.0")

include(Declare.cmake)

add_subdirectory(Vulkan-Headers)
add_subdirectory(Wayland)
add_subdirectory(cpptrace)
add_subdirectory(expat)
add_subdirectory(libdwarf)
add_subdirectory(wl_gena)
add_subdirectory(glslang)

include(CMakePackageConfigHelpers)
configure_package_config_file(Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${DEPS_EXPORT_NAME}Config.cmake
    INSTALL_DESTINATION cmake/${DEPS_EXPORT_NAME}
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${DEPS_EXPORT_NAME}ConfigVersion.cmake
    VERSION ${EXPORT_VERSION}
    COMPATIBILITY ExactVersion
)

set(GENEXPR_BUILD_TYPE $<CONFIG>)
set(GENEXPR_IS_RELEASE $<STREQUAL:${GENEXPR_BUILD_TYPE},Release>)
set(GENEXPR_BUILD_TYPE_PREFIX "$<IF:${GENEXPR_IS_RELEASE},,/${GENEXPR_BUILD_TYPE}>")

foreach(S_TARGET ${STATIC_LIBS})
    install(TARGETS ${PREF}${S_TARGET}.static EXPORT ${DEPS_EXPORT_NAME} DESTINATION "${DEPS_EXPORT_NAME}-${EXPORT_VERSION}/${S_TARGET}/lib${GENEXPR_BUILD_TYPE_PREFIX}")
endforeach()

foreach(D_TARGET ${SHARED_LIBS})
    install(TARGETS ${PREF}${D_TARGET}.shared EXPORT ${DEPS_EXPORT_NAME} DESTINATION "${DEPS_EXPORT_NAME}-${EXPORT_VERSION}/${D_TARGET}/lib${GENEXPR_BUILD_TYPE_PREFIX}")
endforeach()

foreach(H_TARGET ${HEADER_LIBS})
    install(TARGETS ${PREF}${H_TARGET}.headers EXPORT ${DEPS_EXPORT_NAME} FILE_SET headers DESTINATION "${DEPS_EXPORT_NAME}-${EXPORT_VERSION}/${H_TARGET}/include")
endforeach()

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${DEPS_EXPORT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${DEPS_EXPORT_NAME}ConfigVersion.cmake
    DESTINATION
        ${DEPS_EXPORT_NAME}-${EXPORT_VERSION}/cmake
)

install(EXPORT ${DEPS_EXPORT_NAME} DESTINATION "${DEPS_EXPORT_NAME}-${EXPORT_VERSION}/cmake")
