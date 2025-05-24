add_library(${PREF}Launcher.headers INTERFACE)
target_include_directories(${PREF}Launcher.headers INTERFACE include)

add_library(${PREF}Launcher STATIC)
target_sources(${PREF}Launcher PRIVATE TerminateSignal.cc)
target_link_libraries(${PREF}Launcher
    PRIVATE
        ${PREF}Launcher.headers
)

target_sources(${PREF}Launcher PRIVATE CPPTraceLauncher.cc)
target_link_libraries(${PREF}Launcher
    PRIVATE
        ${PREF}cpptrace.headers
        ${PREF}cpptrace.static
)
