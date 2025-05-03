#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include <cstdint>

#include <vulkan/vulkan_raii.hpp>
#include <wayland-client-protocol.h>
#include <xdg-shell.h>

#include "ContextImpl.hh"
#include "WindowImpl.hh"

namespace Wayland {
namespace Impl {

Window::Window(
    std::shared_ptr<Context> context,
    wl_compositor *compositor,
    xdg_wm_base *xdg_base)
    : _context(context)
{
    _h.surface = wl_compositor_create_surface(compositor);
    if (_h.surface == nullptr) {
        throw std::runtime_error("wl_compositor_create_surface() error");
    }

    _h.xdg_surface = xdg_wm_base_get_xdg_surface(xdg_base, _h.surface);
    if (_h.xdg_surface == nullptr) {
        throw std::runtime_error("xdg_wm_base_get_xdg_surface() error");
    }

    xdg_surface_add_listener(
        _h.xdg_surface, &Window::xdg_surface_c_vtable, this);

    _h.top_level = xdg_surface_get_toplevel(_h.xdg_surface);
    if (_h.top_level == nullptr) {
        throw std::runtime_error("xdg_surface_get_toplevel() error");
    }

    wl_surface_commit(_h.surface);

    xdg_toplevel_set_title(_h.top_level, "Vulkan wayland sample");
}

void Window::configure(
    [[maybe_unused]] xdg_surface *xdg_surface, uint32_t serial)
{
    std::cout << "Configure serial " << std::to_string(serial) << '\n';
    xdg_surface_ack_configure(xdg_surface, serial);
}

xdg_surface_listener Window::xdg_surface_c_vtable = []() {
    xdg_surface_listener output{};

    output.configure =
        [](void *data, xdg_surface *xdg_surface, uint32_t serial) {
            Window &window = *reinterpret_cast<Window *>(data);
            window.configure(xdg_surface, serial);
        };

    return output;
}();

} // namespace Impl
} // namespace Wayland
