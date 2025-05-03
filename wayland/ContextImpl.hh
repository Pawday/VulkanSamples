#pragma once

#include <cstdint>

#include <wayland-client-core.h>
#include <xdg-shell.h>

#include <Wayland/Context.hh>
#include <Wayland/Window.hh>

#include "ContextHandles.hh"
#include "RegistryListener.hh"

namespace Wayland {
namespace Impl {

struct ContextImpl
{
    friend struct Wayland::Context;

    using SharedInstance = Wayland::Context::SharedInstance;
    ContextImpl(SharedInstance I);
    ContextImpl(const ContextImpl &) = delete;
    ContextImpl(ContextImpl &&) = default;
    ContextImpl &operator=(const ContextImpl &) = delete;
    ContextImpl &operator=(ContextImpl &&) = default;

    void update()
    {
        wl_display_dispatch(_h.display);
    }

  private:
    ContextHandles _h;
    RegistryListener _registry;

    SharedInstance _instance;

    void xdg_ping(struct xdg_wm_base *xdg_wm_base, uint32_t serial);
    static xdg_wm_base_listener xdg_base_c_vtable;
};

} // namespace Impl
} // namespace Wayland
