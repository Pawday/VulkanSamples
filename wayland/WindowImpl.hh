#pragma once

#include <cstddef>
#include <memory>

#include <cstdint>

#include <vulkan/vulkan_raii.hpp>
#include <wayland-client-protocol.h>
#include <xdg-shell.h>

#include "ContextImpl.hh"
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
template <size_t S>
Window &cast_window(char (&data)[S])
{
    return *reinterpret_cast<Window *>(data);
}
} // namespace

} // namespace Impl
} // namespace Wayland
