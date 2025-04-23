#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <xdg-shell.h>

#include "WaylandContext.hh"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_to_string.hpp"

namespace {

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

struct WaylandContextImpl;

struct WaylandWindowImpl
{
    friend struct ::WaylandContext;

    WaylandWindowImpl(
        std::shared_ptr<WaylandContextImpl> context,
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
            _h.xdg_surface, &WaylandWindowImpl::xdg_surface_c_vtable, this);

        _h.top_level = xdg_surface_get_toplevel(_h.xdg_surface);
        if (_h.top_level == nullptr) {
            throw std::runtime_error("xdg_surface_get_toplevel() error");
        }

        xdg_toplevel_set_title(_h.top_level, "Vulkan wayland sample");
    }

    WaylandWindowImpl(WaylandWindowImpl &&) = default;
    WaylandWindowImpl &operator=(WaylandWindowImpl &&) = default;

    WaylandWindowImpl(const WaylandWindowImpl &) = delete;
    WaylandWindowImpl &operator=(const WaylandWindowImpl &) = delete;

    vk::raii::SurfaceKHR &surface()
    {
        return _vk_surface.value();
    }

  private:
    std::shared_ptr<WaylandContextImpl> _context;
    WindowHandles _h;
    std::optional<vk::raii::SurfaceKHR> _vk_surface;

    void set_surface(vk::raii::SurfaceKHR &&surface)
    {
        if (_vk_surface.has_value()) {
            throw std::runtime_error("Double surface initialisation");
        }
        _vk_surface = std::move(surface);
    }

    void configure([[maybe_unused]] xdg_surface *xdg_surface, uint32_t serial)
    {
        std::cout << "Configure serial" << std::to_string(serial) << '\n';
        xdg_surface_ack_configure(xdg_surface, serial);
        wl_surface_commit(_h.surface);
    }

    static xdg_surface_listener xdg_surface_c_vtable;
};

xdg_surface_listener WaylandWindowImpl::xdg_surface_c_vtable = []() {
    xdg_surface_listener output{};

    output.configure =
        [](void *data, xdg_surface *xdg_surface, uint32_t serial) {
            WaylandWindowImpl &window =
                *reinterpret_cast<WaylandWindowImpl *>(data);
            window.configure(xdg_surface, serial);
        };

    return output;
}();

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

    void reset() noexcept;

    ~ContextHandles()
    {
        reset();
    }

    ContextHandles(const ContextHandles &) = delete;
    ContextHandles &operator=(const ContextHandles &) = delete;

    wl_display *display = nullptr;
    wl_registry *registry = nullptr;
};

struct WaylandContextImpl
{
    friend struct ::WaylandContext;

    using SharedInstance = WaylandContext::SharedInstance;
    WaylandContextImpl(SharedInstance I);
    WaylandContextImpl(const WaylandContextImpl &) = delete;
    WaylandContextImpl(WaylandContextImpl &&) = default;
    WaylandContextImpl &operator=(const WaylandContextImpl &) = delete;
    WaylandContextImpl &operator=(WaylandContextImpl &&) = default;

  private:
    ContextHandles _h;
    RegistryListener _registry;

    SharedInstance _instance;

    void xdg_ping(struct xdg_wm_base *xdg_wm_base, uint32_t serial)
    {
        std::cout << "Ping " << std::to_string(serial) << '\n';
        xdg_wm_base_pong(xdg_wm_base, serial);
    }
    static xdg_wm_base_listener xdg_base_c_vtable;
};

xdg_wm_base_listener WaylandContextImpl::xdg_base_c_vtable = []() {
    xdg_wm_base_listener output{};

    output.ping = [](void *data, xdg_wm_base *xdg_wm_base, uint32_t serial) {
        WaylandContextImpl &ctx = *reinterpret_cast<WaylandContextImpl *>(data);
        ctx.xdg_ping(xdg_wm_base, serial);
    };

    return output;
}();

WaylandContextImpl::WaylandContextImpl(SharedInstance I) : _instance(I)
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
        _registry.xdg_base(), &WaylandContextImpl::xdg_base_c_vtable, this);

    wl_display_roundtrip(_h.display);

    for (auto &iface_name : _registry.interface_names()) {
        std::cout << "Found wl_interface \"" << iface_name << "\"\n";
    }

    wl_display_roundtrip(_h.display);
}

void ContextHandles::reset() noexcept
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

struct WaylandContextImplShared
{
    using SharedInstance = WaylandContext::SharedInstance;
    WaylandContextImplShared(SharedInstance I)
        : _(std::make_shared<WaylandContextImpl>(I))
    {
    }

    operator std::shared_ptr<WaylandContextImpl>()
    {
        return _;
    }

    WaylandContextImpl *operator->()
    {
        return _.get();
    }

  private:
    std::shared_ptr<WaylandContextImpl> _;
};

template <size_t S>
WaylandContextImplShared &ctx_impl(char (&data)[S])
{
    return *reinterpret_cast<WaylandContextImplShared *>(data);
}

template <size_t S>
WaylandWindowImpl &win_impl(char (&data)[S])
{
    return *reinterpret_cast<WaylandWindowImpl *>(data);
}

} // namespace

WaylandContext::WaylandContext(SharedInstance I)
{
    static_assert(alignof(WaylandContext) >= alignof(WaylandContextImplShared));
    static_assert(sizeof(WaylandContext) >= sizeof(WaylandContextImplShared));
    new (_) WaylandContextImplShared{I};
}

WaylandContext::WaylandContext(WaylandContext &&o)
{
    new (_) WaylandContextImplShared{std::move(ctx_impl(o._))};
}

WaylandContext &WaylandContext::operator=(WaylandContext &&o)
{
    ctx_impl(_).operator=(std::move(ctx_impl(o._)));
    return *this;
}

WaylandContext::~WaylandContext()
{
    ctx_impl(_).~WaylandContextImplShared();
}

WaylandWindow::WaylandWindow() = default;

WaylandWindow::WaylandWindow(WaylandWindow &&o)
{
    new (_) WaylandWindowImpl{std::move(win_impl(o._))};
};

WaylandWindow &WaylandWindow::operator=(WaylandWindow &&o)
{
    win_impl(_).operator=(std::move(win_impl(o._)));
    return *this;
}

WaylandWindow::~WaylandWindow()
{
    win_impl(_).~WaylandWindowImpl();
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

WaylandWindow WaylandContext::create_window()
{
    WaylandContextImplShared &ctx = ctx_impl(_);

    auto vkCreateWaylandSurfaceKHR =
        reinterpret_cast<vkCreateWaylandSurfaceKHR_PFN>(
            ctx->_instance->getProcAddr("vkCreateWaylandSurfaceKHR"));
    if (vkCreateWaylandSurfaceKHR == nullptr) {
        throw std::runtime_error("Cannot load vkCreateWaylandSurfaceKHR()");
    }

    WaylandWindow output{};

    WaylandWindowImpl win{
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

    new (output._) WaylandWindowImpl(std::move(win));

    return output;
}

vk::raii::SurfaceKHR &WaylandWindow::surface()
{
    return win_impl(_).surface();
}

