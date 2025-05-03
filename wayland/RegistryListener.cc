#include <iostream>
#include <string>

#include <cstdint>

#include <wayland-client-protocol.h>
#include <xdg-shell.h>

#include "RegistryListener.hh"

namespace Wayland {
namespace Impl {

void RegistryListener::add(
    wl_registry *wl_registry,
    uint32_t name,
    const char *interface_cstr,
    uint32_t version)
{
    const std::string interface{interface_cstr};
    _interface_names.push_back(interface);

    if (interface == wl_compositor_interface.name) {
        void *compositor_p = wl_registry_bind(
            wl_registry, name, &wl_compositor_interface, version);
        if (compositor_p == nullptr) {
            std::string msg;
            msg += "Cannot bind " + interface + " v" + std::to_string(version);
            std::cout << msg << '\n';
        } else {
            _compositor = reinterpret_cast<wl_compositor *>(compositor_p);
        }
    }

    if (interface == xdg_wm_base_interface.name) {
        void *xdg_base_p = wl_registry_bind(
            wl_registry, name, &xdg_wm_base_interface, version);
        if (xdg_base_p == nullptr) {
            std::string msg;
            msg += "Cannot bind " + interface + " v" + std::to_string(version);
            std::cout << msg << '\n';
        } else {
            _xdg_base = reinterpret_cast<xdg_wm_base *>(xdg_base_p);
        }
    }
}

RegistryListener::~RegistryListener()
{
    if (_xdg_base.has_value()) {
        xdg_wm_base_destroy(_xdg_base.value());
        _xdg_base.reset();
    }

    if (_compositor.has_value()) {
        wl_compositor_destroy(_compositor.value());
        _compositor.reset();
    }
}

wl_registry_listener RegistryListener::c_vtable = []() {
    wl_registry_listener output{};
    output.global = [](void *data,
                       wl_registry *wl_registry,
                       uint32_t name,
                       const char *interface,
                       uint32_t version) {
        RegistryListener &l = *reinterpret_cast<RegistryListener *>(data);
        l.add(wl_registry, name, interface, version);
    };
    output.global_remove =
        [](void *data, wl_registry *wl_registry, uint32_t name) {
            RegistryListener &l = *reinterpret_cast<RegistryListener *>(data);
            l.remove(wl_registry, name);
        };
    return output;
}();

} // namespace Impl
} // namespace Wayland
