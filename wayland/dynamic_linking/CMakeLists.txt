target_sources(${PREF}WaylandContext PRIVATE
    Context.cc
    Window.cc
    VulkanWindow.cc
)

target_compile_options(${PREF}WaylandContext PRIVATE "-Wno-unused-private-field")
