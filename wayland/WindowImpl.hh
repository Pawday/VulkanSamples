#pragma once

#include <memory>
#include <optional>

#include <cstdint>

#include <vulkan/vulkan_raii.hpp>
#include <wayland-client-protocol.h>
#include <xdg-shell.h>

#include <Wayland/Context.hh>

#include "ContextImpl.hh"
#include "WindowHandles.hh"

namespace Wayland {
namespace Impl {

struct Window
{
    friend struct Wayland::Context;

    Window(
        std::shared_ptr<ContextImpl> context,
        wl_compositor *compositor,
        xdg_wm_base *xdg_base);

    Window(Window &&) = default;
    Window &operator=(Window &&) = default;

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    vk::raii::SurfaceKHR &surface()
    {
        return _vk_surface.value();
    }

  private:
    std::shared_ptr<ContextImpl> _context;
    WindowHandles _h;
    std::optional<vk::raii::SurfaceKHR> _vk_surface;

    void set_surface(vk::raii::SurfaceKHR &&surface);

    void configure(xdg_surface *xdg_surface, uint32_t serial);

    static xdg_surface_listener xdg_surface_c_vtable;
};

} // namespace Impl
} // namespace Wayland
