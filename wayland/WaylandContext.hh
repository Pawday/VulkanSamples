#pragma once

#include <memory>
#include <vulkan/vulkan_raii.hpp>

namespace Wayland {
struct WaylandWindow;

struct WaylandContext
{
    using SharedInstance = std::shared_ptr<vk::raii::Instance>;
    WaylandContext(SharedInstance I);
    WaylandContext(WaylandContext &&);
    WaylandContext &operator=(WaylandContext &&);
    ~WaylandContext();

    WaylandWindow create_window();

    void update();

    WaylandContext(const WaylandContext &) = delete;
    WaylandContext &operator=(const WaylandContext &) = delete;

  private:
    alignas(8) char _[16];
};

struct WaylandWindow
{
    WaylandWindow(WaylandWindow &&);
    WaylandWindow &operator=(WaylandWindow &&);
    ~WaylandWindow();

    WaylandWindow(const WaylandWindow &) = delete;
    WaylandWindow &operator=(const WaylandWindow &) = delete;

    vk::raii::SurfaceKHR &surface();

  private:
    friend struct WaylandContext;
    WaylandWindow();
    alignas(16) char _[128];
};

} // namespace Wayland
