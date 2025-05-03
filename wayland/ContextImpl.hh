#pragma once

#include <cstddef>
#include <cstdint>

#include <memory>
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
    ContextImpl();
    ContextImpl(const ContextImpl &) = delete;
    ContextImpl(ContextImpl &&) = default;
    ContextImpl &operator=(const ContextImpl &) = delete;
    ContextImpl &operator=(ContextImpl &&) = default;

    void update()
    {
        wl_display_dispatch(_h.display);
    }

    ContextHandles _h;
    RegistryListener _registry;

    void xdg_ping(struct xdg_wm_base *xdg_wm_base, uint32_t serial);
    static xdg_wm_base_listener xdg_base_c_vtable;
};

struct ContextShared
{
    ContextShared() : _(std::make_shared<ContextImpl>())
    {
    }

    operator std::shared_ptr<ContextImpl>()
    {
        return _;
    }

    ContextImpl *operator->()
    {
        return _.get();
    }

  private:
    std::shared_ptr<ContextImpl> _;
};

namespace {
template <size_t S>
ContextShared &cast_context(char (&data)[S])
{
    return *reinterpret_cast<ContextShared *>(data);
}

} // namespace

} // namespace Impl
} // namespace Wayland
