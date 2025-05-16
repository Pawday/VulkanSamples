#pragma once

#include <memory>

#include <cstddef>
#include <cstdint>

#include <wayland-client-core.h>
#include <xdg-shell.h>

#include "ContextHandles.hh"
#include "RegistryListener.hh"
#include "Wayland/Context.hh"

namespace Wayland {
namespace Impl {

struct Context
{
    Context();
    Context(const Context &) = delete;
    Context(Context &&) = default;
    Context &operator=(const Context &) = delete;
    Context &operator=(Context &&) = default;

    void update()
    {
        wl_display_dispatch_pending(_h.display);
    }

    ContextHandles _h;
    RegistryListener _registry;

    void xdg_ping(struct xdg_wm_base *xdg_wm_base, uint32_t serial);
    static xdg_wm_base_listener xdg_base_c_vtable;
};

struct ContextShared
{
    ContextShared() : _(std::make_shared<Context>())
    {
    }

    operator std::shared_ptr<Context>()
    {
        return _;
    }

    Context *operator->()
    {
        return _.get();
    }

  private:
    std::shared_ptr<Context> _;
};

namespace {
inline ContextShared &cast_context(Wayland::Context::ImplT &impl)
{
    return *reinterpret_cast<ContextShared *>(impl());
}

} // namespace

} // namespace Impl
} // namespace Wayland
