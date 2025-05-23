add_library(${PREF}Launcher.headers INTERFACE)
target_include_directories(${PREF}Launcher.headers INTERFACE include)

add_library(${PREF}Launcher STATIC)
target_link_libraries(${PREF}Launcher
    PRIVATE
        ${PREF}Launcher.headers
        ${PREF}cpptrace.headers
        ${PREF}cpptrace.static
)

target_sources(${PREF}Launcher PRIVATE CPPTraceLauncher.cc)
