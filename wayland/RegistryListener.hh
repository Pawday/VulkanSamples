#pragma once

#include <optional>
#include <string>
#include <vector>

#include <cstdint>

#include <wayland-client-protocol.h>
#include <xdg-shell.h>

namespace Wayland {
namespace Impl {

struct RegistryListener
{
    RegistryListener(const RegistryListener &) = delete;
    RegistryListener &operator=(const RegistryListener &) = delete;

    RegistryListener() = default;
    RegistryListener(RegistryListener &&) = default;
    RegistryListener &operator=(RegistryListener &&) = default;
    ~RegistryListener();

    wl_compositor *compositor()
    {
        return _compositor.value();
    }

    xdg_wm_base *xdg_base()
    {
        return _xdg_base.value();
    }

    const std::vector<std::string> &interface_names() const
    {
        return _interface_names;
    }

    static wl_registry_listener c_vtable;

  private:
    std::vector<std::string> _interface_names;

    std::optional<wl_compositor *> _compositor;
    std::optional<xdg_wm_base *> _xdg_base;

    void
        add(wl_registry *wl_registry,
            uint32_t name,
            const char *interface_cstr,
            uint32_t version);

    void remove(
        [[maybe_unused]] wl_registry *wl_registry,
        [[maybe_unused]] uint32_t name)
    {
    }
};

} // namespace Impl
} // namespace Wayland
