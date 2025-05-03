#pragma once

#include <memory>
#include <utility>

#include <wayland-client-protocol.h>
#include <xdg-shell.h>

namespace Wayland {
namespace Impl {

struct WindowHandles
{
    WindowHandles() = default;

    WindowHandles(WindowHandles &&o)
        : surface(o.surface), xdg_surface(o.xdg_surface), top_level(o.top_level)
    {
        o.surface = nullptr;
        o.xdg_surface = nullptr;
        o.top_level = nullptr;
    }

    WindowHandles &operator=(WindowHandles &&o)
    {
        if (this == ::std::addressof(o)) {
            return *this;
        }

        std::swap(surface, o.surface);
        std::swap(xdg_surface, o.xdg_surface);
        std::swap(top_level, o.top_level);

        return *this;
    }

    void reset()
    {
        if (top_level) {
            xdg_toplevel_destroy(top_level);
            top_level = nullptr;
        }

        if (xdg_surface) {
            xdg_surface_destroy(xdg_surface);
            xdg_surface = nullptr;
        }

        if (surface) {
            wl_surface_destroy(surface);
            surface = nullptr;
        }
    }

    ~WindowHandles()
    {
        reset();
    }

    WindowHandles(const WindowHandles &) = delete;
    WindowHandles &operator=(const WindowHandles &) = delete;

    wl_surface *surface = nullptr;
    xdg_surface *xdg_surface = nullptr;
    xdg_toplevel *top_level = nullptr;
};

} // namespace Impl
} // namespace Wayland
