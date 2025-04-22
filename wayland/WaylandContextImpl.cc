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

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <xdg-shell.h>

namespace {

struct RegistryListener
{
    RegistryListener(const RegistryListener &) = delete;
    RegistryListener &operator=(const RegistryListener &) = delete;

    RegistryListener() = default;
    RegistryListener(RegistryListener &&) = default;
    RegistryListener &operator=(RegistryListener &&) = default;
    ~RegistryListener();

    static wl_registry_listener c_vtable;
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

  private:
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

    std::vector<std::string> _interface_names;

    std::optional<wl_compositor *> _compositor;
    std::optional<xdg_wm_base *> _xdg_base;
};

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

struct Handles
{
    Handles() = default;
    Handles(const Handles &) = delete;
    Handles &operator=(const Handles &) = delete;

    Handles(Handles &&o) noexcept
        : display(o.display), registry(o.registry), surface(o.surface),
          xdg_surface(o.xdg_surface), top_level(o.top_level)
    {
        o.display = nullptr;
        o.registry = nullptr;
        o.surface = nullptr;
        o.xdg_surface = nullptr;
        o.top_level = nullptr;
    }
    Handles &operator=(Handles &&o) noexcept
    {
        if (this == ::std::addressof(o)) {
            return *this;
        }

        std::swap(display, o.display);
        std::swap(registry, o.registry);
        std::swap(surface, o.surface);
        std::swap(xdg_surface, o.xdg_surface);
        std::swap(top_level, o.top_level);

        return *this;
    }

    void reset() noexcept;

    ~Handles()
    {
        reset();
    }

    wl_display *display = nullptr;
    wl_registry *registry = nullptr;
    wl_surface *surface = nullptr;
    xdg_surface *xdg_surface = nullptr;
    xdg_toplevel *top_level = nullptr;
};

struct WaylandContextImpl
{
    void xdg_ping(struct xdg_wm_base *xdg_wm_base, uint32_t serial)
    {
        std::cout << "Ping " << std::to_string(serial) << '\n';
        xdg_wm_base_pong(xdg_wm_base, serial);
    }
    static xdg_wm_base_listener xdg_base_c_vtable;

    void configure(
        [[maybe_unused]] struct xdg_surface *xdg_surface, uint32_t serial)
    {
        std::cout << "Configure serial" << std::to_string(serial) << '\n';
        xdg_surface_ack_configure(xdg_surface, serial);
        wl_surface_commit(_h.surface);
    }

    static xdg_surface_listener xdg_surface_c_vtable;

    WaylandContextImpl();
    WaylandContextImpl(const WaylandContextImpl &) = delete;
    WaylandContextImpl(WaylandContextImpl &&) = delete;
    WaylandContextImpl &operator=(const WaylandContextImpl &) = delete;
    WaylandContextImpl &operator=(WaylandContextImpl &&) = delete;

    void update()
    {
        auto nb_events = wl_display_dispatch(_h.display);
        if (nb_events < 0) {
            auto status = errno;
            std::string message = "wl_display_dispatch() failure: ";
            message += "code " + std::to_string(status) + ' ';
            message = message + "(" + strerror(status) + ')';
            std::cout << message << '\n';
            return;
        }

        std::cout << "events: " << std::to_string(nb_events) << '\n';
    }

    bool need_close() const
    {
        return _need_close;
    }

  private:
    Handles _h;
    RegistryListener _registry;

    bool _need_close = false;
};

xdg_wm_base_listener WaylandContextImpl::xdg_base_c_vtable = []() {
    xdg_wm_base_listener output{};

    output.ping = [](void *data,
                     struct xdg_wm_base *xdg_wm_base,
                     uint32_t serial) {
        WaylandContextImpl &ctx = *reinterpret_cast<WaylandContextImpl *>(data);
        ctx.xdg_ping(xdg_wm_base, serial);
    };

    return output;
}();

xdg_surface_listener WaylandContextImpl::xdg_surface_c_vtable = []() {
    xdg_surface_listener output{};

    output.configure = [](void *data,
                          struct xdg_surface *xdg_surface,
                          uint32_t serial) {
        WaylandContextImpl &ctx = *reinterpret_cast<WaylandContextImpl *>(data);
        ctx.configure(xdg_surface, serial);
    };

    return output;
}();

WaylandContextImpl::WaylandContextImpl()
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

    for (auto &iface_name : _registry.interface_names()) {
        std::cout << "Found wl_interface \"" << iface_name << "\"\n";
    }

    _h.surface = wl_compositor_create_surface(_registry.compositor());
    if (_h.surface == nullptr) {
        throw std::runtime_error("wl_compositor_create_surface() error");
    }

    xdg_wm_base_add_listener(
        _registry.xdg_base(), &WaylandContextImpl::xdg_base_c_vtable, this);

    _h.xdg_surface =
        xdg_wm_base_get_xdg_surface(_registry.xdg_base(), _h.surface);
    if (_h.xdg_surface == nullptr) {
        throw std::runtime_error("xdg_wm_base_get_xdg_surface() error");
    }

    xdg_surface_add_listener(
        _h.xdg_surface, &WaylandContextImpl::xdg_surface_c_vtable, this);

    _h.top_level = xdg_surface_get_toplevel(_h.xdg_surface);
    if (_h.top_level == nullptr) {
        throw std::runtime_error("xdg_surface_get_toplevel() error");
    }

    xdg_toplevel_set_title(_h.top_level, "Vulkan wayland sample");

    wl_display_roundtrip(_h.display);
}

void Handles::reset() noexcept
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

    if (registry) {
        wl_registry_destroy(registry);
        registry = nullptr;
    }

    if (display) {
        wl_display_disconnect(display);
        display = nullptr;
    }
}

} // namespace
