#include <iostream>
#include <memory>
#include <optional>
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

#include "Context.hh"
#include "ContextImpl.hh"
#include "RegistryListener.hh"
#include "WindowImpl.hh"

namespace Wayland {
namespace Impl {

xdg_wm_base_listener ContextImpl::xdg_base_c_vtable = []() {
    xdg_wm_base_listener output{};

    output.ping = [](void *data, xdg_wm_base *xdg_wm_base, uint32_t serial) {
        ContextImpl &ctx = *reinterpret_cast<ContextImpl *>(data);
        ctx.xdg_ping(xdg_wm_base, serial);
    };

    return output;
}();

ContextImpl::ContextImpl(SharedInstance I) : _instance(I)
{
    if (_instance == nullptr) {
        throw std::runtime_error("vkInstance is null");
    }

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
        _registry.xdg_base(), &ContextImpl::xdg_base_c_vtable, this);

    wl_display_roundtrip(_h.display);

    for (auto &iface_name : _registry.interface_names()) {
        std::cout << "Found wl_interface \"" << iface_name << "\"\n";
    }

    wl_display_roundtrip(_h.display);
}

void ContextImpl::xdg_ping(struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
    std::cout << "Ping " << std::to_string(serial) << '\n';
    xdg_wm_base_pong(xdg_wm_base, serial);
}

struct ContextShared
{
    using SharedInstance = Wayland::Context::SharedInstance;
    ContextShared(SharedInstance I) : _(std::make_shared<ContextImpl>(I))
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

template <size_t S>
Window &cast_window(char (&data)[S])
{
    return *reinterpret_cast<Window *>(data);
}
} // namespace

} // namespace Impl
} // namespace Wayland

namespace Wayland {

Context::Context(SharedInstance I)
{
    static_assert(alignof(Context) >= alignof(Impl::ContextShared));
    static_assert(sizeof(Context) >= sizeof(Impl::ContextShared));
    new (_) Impl::ContextShared{I};
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

// Provided by VK_KHR_wayland_surface
using VkWaylandSurfaceCreateFlagsKHR = VkFlags;

struct VkWaylandSurfaceCreateInfoKHR
{
    VkStructureType sType;
    const void *pNext;
    VkWaylandSurfaceCreateFlagsKHR flags;
    struct wl_display *display;
    struct wl_surface *surface;
};

using vkCreateWaylandSurfaceKHR_PFN = VkResult (*)(
    VkInstance instance,
    const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkSurfaceKHR *pSurface);

Window Context::create_window()
{
    Impl::ContextShared &ctx = Impl::cast_context(_);

    auto vkCreateWaylandSurfaceKHR =
        reinterpret_cast<vkCreateWaylandSurfaceKHR_PFN>(
            ctx->_instance->getProcAddr("vkCreateWaylandSurfaceKHR"));
    if (vkCreateWaylandSurfaceKHR == nullptr) {
        throw std::runtime_error("Cannot load vkCreateWaylandSurfaceKHR()");
    }

    Impl::Window win{
        ctx, ctx->_registry.compositor(), ctx->_registry.xdg_base()};

    VkSurfaceKHR c_surface{};

    VkWaylandSurfaceCreateInfoKHR ci{};
    ci.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    ci.display = ctx->_h.display;
    ci.surface = win._h.surface;

    vk::raii::Instance &raii_instance = *ctx->_instance;
    vk::Instance i{raii_instance};
    vk::Result res{vkCreateWaylandSurfaceKHR(i, &ci, nullptr, &c_surface)};

    if (res != vk::Result::eSuccess) {
        std::string message = "Cannot create vulkang-wayland surface";
        message = message + " status " + vk::to_string(res);
        throw std::runtime_error(message);
    }

    vk::raii::SurfaceKHR cxx_surface{raii_instance, c_surface};
    win.set_surface(std::move(cxx_surface));

    Window output{};

    static_assert(sizeof(Window) >= sizeof(Impl::Window));
    static_assert(alignof(Window) >= alignof(Impl::Window));
    new (output._) Impl::Window(std::move(win));
    return output;
}

vk::raii::SurfaceKHR &Window::surface()
{
    return Impl::cast_window(_).surface();
}

void Context::update()
{
    Impl::cast_context(_)->update();
};

} // namespace Wayland
