#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_to_string.hpp>

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <xdg-shell.h>

#include <Wayland/Context.hh>
#include <Wayland/Window.hh>

#include "ContextImpl.hh"
#include "RegistryListener.hh"
#include "WindowImpl.hh"

namespace Wayland {
namespace Impl {

xdg_wm_base_listener Context::xdg_base_c_vtable = []() {
    xdg_wm_base_listener output{};

    output.ping = [](void *data, xdg_wm_base *xdg_wm_base, uint32_t serial) {
        Context &ctx = *reinterpret_cast<Context *>(data);
        ctx.xdg_ping(xdg_wm_base, serial);
    };

    return output;
}();

Context::Context()
{
    _h.display = wl_display_connect(nullptr);
    if (_h.display == nullptr) {
        throw std::runtime_error("wl_display_connect() error");
    }

    _h.registry = wl_display_get_registry(_h.display);
    if (_h.registry == nullptr) {
        throw std::runtime_error("wl_display_get_registry() error");
    }

    auto add_listener_status = wl_registry_add_listener(
        _h.registry, &RegistryListener::c_vtable, std::addressof(_registry));
    if (add_listener_status != 0) {
        throw std::runtime_error("wl_registry_add_listener() error");
    }

    wl_display_roundtrip(_h.display);

    xdg_wm_base_add_listener(
        _registry.xdg_base(), &Context::xdg_base_c_vtable, this);

    wl_display_roundtrip(_h.display);

    for (auto &iface_name : _registry.interface_names()) {
        std::cout << "Found wl_interface \"" << iface_name << "\"\n";
    }

    wl_display_roundtrip(_h.display);
}

void Context::xdg_ping(struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
    std::cout << "Ping " << std::to_string(serial) << '\n';
    xdg_wm_base_pong(xdg_wm_base, serial);
}

} // namespace Impl
} // namespace Wayland

namespace Wayland {

Context::Context()
{
    static_assert(alignof(Context) >= alignof(Impl::ContextShared));
    static_assert(sizeof(Context) >= sizeof(Impl::ContextShared));
    new (_) Impl::ContextShared;
}

Context::Context(Context &&o)
{
    new (_) Impl::ContextShared{std::move(Impl::cast_context(o._))};
}

Context &Context::operator=(Context &&o)
{
    Impl::cast_context(_).operator=(std::move(Impl::cast_context(o._)));
    return *this;
}

Context::~Context()
{
    Impl::cast_context(_).~ContextShared();
}

Window::Window() = default;

Window::Window(Window &&o)
{
    new (_) Impl::Window{std::move(Impl::cast_window(o._))};
};

Window &Window::operator=(Window &&o)
{
    Impl::cast_window(_).operator=(std::move(Impl::cast_window(o._)));
    return *this;
}

Window::~Window()
{
    Impl::cast_window(_).~Window();
}

Window Context::create_window()
{
    Impl::ContextShared &ctx = Impl::cast_context(_);

    Impl::Window win{
        ctx, ctx->_registry.compositor(), ctx->_registry.xdg_base()};

    Window output{};
    static_assert(sizeof(Window) >= sizeof(Impl::Window));
    static_assert(alignof(Window) >= alignof(Impl::Window));
    new (output._) Impl::Window(std::move(win));
    return output;
}

void Context::update()
{
    Impl::cast_context(_)->update();
};

} // namespace Wayland
