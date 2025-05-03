#pragma once

#include <memory>
#include <utility>

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

namespace Wayland {
namespace Impl {

struct ContextHandles
{
    ContextHandles() = default;

    ContextHandles(ContextHandles &&o) noexcept
        : display(o.display), registry(o.registry)
    {
        o.display = nullptr;
        o.registry = nullptr;
    }
    ContextHandles &operator=(ContextHandles &&o) noexcept
    {
        if (this == ::std::addressof(o)) {
            return *this;
        }

        std::swap(display, o.display);
        std::swap(registry, o.registry);

        return *this;
    }

    void reset() noexcept
    {
        if (registry) {
            wl_registry_destroy(registry);
            registry = nullptr;
        }

        if (display) {
            wl_display_disconnect(display);
            display = nullptr;
        }
    }

    ~ContextHandles()
    {
        reset();
    }

    ContextHandles(const ContextHandles &) = delete;
    ContextHandles &operator=(const ContextHandles &) = delete;

    wl_display *display = nullptr;
    wl_registry *registry = nullptr;
};

} // namespace Impl
} // namespace Wayland
