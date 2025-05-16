#pragma once

#include <cstddef>
#include <memory>

#include <cstdint>

#include <vulkan/vulkan_raii.hpp>
#include <wayland-client-protocol.h>
#include <xdg-shell.h>

#include "ContextImpl.hh"
#include "Wayland/Window.hh"
#include "WindowHandles.hh"

namespace Wayland {
namespace Impl {

struct Window
{
    Window(
        std::shared_ptr<Context> context,
        wl_compositor *compositor,
        xdg_wm_base *xdg_base);

    Window(Window &&) = default;
    Window &operator=(Window &&) = default;

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    std::shared_ptr<Context> _context;
    WindowHandles _h;

    void configure(xdg_surface *xdg_surface, uint32_t serial);

  private:
    static xdg_surface_listener xdg_surface_c_vtable;
};

namespace {
inline Window &cast_window(Wayland::Window::ImplT &impl)
{
    return *reinterpret_cast<Window *>(impl());
}
} // namespace

} // namespace Impl
} // namespace Wayland
