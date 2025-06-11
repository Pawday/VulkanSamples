set(EXPORT_VERSION "0.0.0")

include(Declare.cmake)

application_sources(${PREF}host_glslc PRIVATE glslc.cc)
application_link_libraries(${PREF}host_glslc
    PRIVATE
        ${PREF}glslang.headers
    INTERFACE
        ${PREF}glslang.static
)

application_sources(${PREF}host_wl_gena PRIVATE wl_gena.cc)
application_link_libraries(${PREF}host_wl_gena
    PRIVATE
        ${PREF}wl_gena.headers
    INTERFACE
        ${PREF}wl_gena.static
        ${PREF}expat.static
)

include(CMakePackageConfigHelpers)
configure_package_config_file(Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${HOSTTOOLS_EXPORT_NAME}Config.cmake
    INSTALL_DESTINATION cmake/${HOSTTOOLS_EXPORT_NAME}
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${HOSTTOOLS_EXPORT_NAME}ConfigVersion.cmake
    VERSION ${EXPORT_VERSION}
    COMPATIBILITY ExactVersion
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${HOSTTOOLS_EXPORT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${HOSTTOOLS_EXPORT_NAME}ConfigVersion.cmake
    DESTINATION
        ${HOSTTOOLS_EXPORT_NAME}-${EXPORT_VERSION}/cmake
)

foreach(TOOL ${TOOLS})
    application_install(${PREF}host_${TOOL}
        EXPORT ${HOSTTOOLS_EXPORT_NAME}
        DESTINATION "${HOSTTOOLS_EXPORT_NAME}-${EXPORT_VERSION}/${TOOL}/bin"
    )
endforeach()

install(EXPORT ${HOSTTOOLS_EXPORT_NAME} DESTINATION "${HOSTTOOLS_EXPORT_NAME}-${EXPORT_VERSION}/cmake")
