set(EXPORT_VERSION "0.0.0")


target_sources(${PREF}host_TestHostApplication PRIVATE TestHostApplication.cc)



include(CMakePackageConfigHelpers)
configure_package_config_file(Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${HOST_APPS_EXPORT_NAME}Config.cmake
    INSTALL_DESTINATION cmake/${HOST_APPS_EXPORT_NAME}
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${HOST_APPS_EXPORT_NAME}ConfigVersion.cmake
    VERSION ${EXPORT_VERSION}
    COMPATIBILITY ExactVersion
)

foreach(HOST_APPLICATION ${HOST_APPLICATIONS})
    set(TGT "${PREF}host_${HOST_APPLICATION}")
    install_application(${TGT} EXPORT ${HOST_APPS_EXPORT_NAME} DESTINATION "${HOST_APPS_EXPORT_NAME}-${EXPORT_VERSION}/bin/")
endforeach()

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${HOST_APPS_EXPORT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${HOST_APPS_EXPORT_NAME}ConfigVersion.cmake
    DESTINATION
        ${HOST_APPS_EXPORT_NAME}-${EXPORT_VERSION}/cmake
)


install(EXPORT ${HOST_APPS_EXPORT_NAME} DESTINATION "${HOST_APPS_EXPORT_NAME}-${EXPORT_VERSION}/cmake")

foreach(S_TARGET ${STATIC_LIBS})
endforeach()

