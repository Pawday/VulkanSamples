set(EXPORT_VERSION "0.0.0")

#add_subdirectory(wl_gena)

include(CMakePackageConfigHelpers)
configure_package_config_file(Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${HOSTBOX_EXPORT_NAME}Config.cmake
    INSTALL_DESTINATION cmake/${HOSTBOX_EXPORT_NAME}
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${HOSTBOX_EXPORT_NAME}ConfigVersion.cmake
    VERSION ${EXPORT_VERSION}
    COMPATIBILITY ExactVersion
)

foreach(HOST_APPLICATION ${HOST_APPLICATIONS})
    set(TGT "${PREF}${HOST_APPLICATION}")
    install(TARGETS ${TGT} EXPORT ${HOSTBOX_EXPORT_NAME} DESTINATION "${HOSTBOX_EXPORT_NAME}-${EXPORT_VERSION}/bin/")
endforeach()

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${HOSTBOX_EXPORT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${HOSTBOX_EXPORT_NAME}ConfigVersion.cmake
    DESTINATION
        ${HOSTBOX_EXPORT_NAME}-${EXPORT_VERSION}/cmake
)

install(TARGETS ${PREF}HostBox EXPORT ${HOSTBOX_EXPORT_NAME})
install(EXPORT ${HOSTBOX_EXPORT_NAME} DESTINATION "${HOSTBOX_EXPORT_NAME}-${EXPORT_VERSION}/cmake")

