#include <iostream>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include <dlfcn.h>

#include "ClientCoreLibrary.hh"

namespace Wayland {

namespace {

struct ClientCoreLibraryPosix
{
    ClientCoreLibraryPosix(const std::string &library_name)
    {
        dlerror();
        void *handle = dlopen(library_name.c_str(), RTLD_NOW | RTLD_LOCAL);
        const char *status = dlerror();

        if (status) {
            throw std::runtime_error(status);
        }
        _handle = handle;
    }

    ClientCoreLibraryPosix(ClientCoreLibraryPosix &&o) : _handle(o._handle)
    {
        o._handle = nullptr;
    }

    ClientCoreLibraryPosix &operator=(ClientCoreLibraryPosix &&o)
    {
        std::swap(_handle, o._handle);
        return *this;
    }

    ~ClientCoreLibraryPosix()
    {
        if (_handle) {
            dlclose(_handle);
        }
    }

    template <typename PFN_T>
    PFN_T load(const char *name)
    {
        void *sym = dlsym(_handle, name);
        const char *status = dlerror();
        if (status) {
            std::cout << "[WARN] " << status << '\n';
            return nullptr;
        }

        return reinterpret_cast<PFN_T>(sym);
    }

    ClientCoreLibraryPosix(const ClientCoreLibraryPosix &) = delete;
    ClientCoreLibraryPosix &operator=(const ClientCoreLibraryPosix &) = delete;

  private:
    void *_handle = nullptr;
};

ClientCoreLibraryPosix &cast(ClientCoreLibrary::ImplT &impl)
{
    return *reinterpret_cast<ClientCoreLibraryPosix *>(impl());
}

} // namespace

ClientCoreLibrary::ClientCoreLibrary()
{
    ClientCoreLibraryPosix lib{"libwayland-client.so"};

#define LOAD(fn_name) fn_name = lib.load<decltype(fn_name)>(#fn_name)
    LOAD(wl_event_queue_destroy);
    LOAD(wl_proxy_marshal_flags);
    LOAD(wl_proxy_marshal_array_flags);
    LOAD(wl_proxy_marshal);
    LOAD(wl_proxy_marshal_array);
    LOAD(wl_proxy_create);
    LOAD(wl_proxy_create_wrapper);
    LOAD(wl_proxy_wrapper_destroy);
    LOAD(wl_proxy_marshal_constructor);
    LOAD(wl_proxy_marshal_constructor_versioned);
    LOAD(wl_proxy_marshal_array_constructor);
    LOAD(wl_proxy_marshal_array_constructor_versioned);
    LOAD(wl_proxy_destroy);
    LOAD(wl_proxy_add_listener);
    LOAD(wl_proxy_get_listener);
    LOAD(wl_proxy_add_dispatcher);
    LOAD(wl_proxy_set_user_data);
    LOAD(wl_proxy_get_user_data);
    LOAD(wl_proxy_get_version);
    LOAD(wl_proxy_get_id);
    LOAD(wl_proxy_set_tag);
    LOAD(wl_proxy_get_tag);
    LOAD(wl_proxy_get_class);
    LOAD(wl_proxy_get_display);
    LOAD(wl_proxy_set_queue);
    LOAD(wl_proxy_get_queue);
    LOAD(wl_event_queue_get_name);
    LOAD(wl_display_connect);
    LOAD(wl_display_connect_to_fd);
    LOAD(wl_display_disconnect);
    LOAD(wl_display_get_fd);
    LOAD(wl_display_dispatch);
    LOAD(wl_display_dispatch_queue);
    LOAD(wl_display_dispatch_timeout);
    LOAD(wl_display_dispatch_queue_timeout);
    LOAD(wl_display_dispatch_queue_pending);
    LOAD(wl_display_dispatch_pending);
    LOAD(wl_display_get_error);
    LOAD(wl_display_get_protocol_error);
    LOAD(wl_display_flush);
    LOAD(wl_display_roundtrip_queue);
    LOAD(wl_display_roundtrip);
    LOAD(wl_display_create_queue);
    LOAD(wl_display_create_queue_with_name);
    LOAD(wl_display_prepare_read_queue);
    LOAD(wl_display_prepare_read);
    LOAD(wl_display_cancel_read);
    LOAD(wl_display_read_events);
    LOAD(wl_log_set_handler_client);
    LOAD(wl_display_set_max_buffer_size);
#undef LOAD

    static_assert(
        sizeof(ClientCoreLibraryPosix) <= sizeof(ClientCoreLibrary::ImplT));
    static_assert(
        alignof(ClientCoreLibraryPosix) <= alignof(ClientCoreLibrary::ImplT));
    new (impl()) ClientCoreLibraryPosix{std::move(lib)};
}

ClientCoreLibrary::ClientCoreLibrary(ClientCoreLibrary &&o)
{
    new (impl()) ClientCoreLibraryPosix{std::move(cast(o.impl))};
}

ClientCoreLibrary &ClientCoreLibrary::operator=(ClientCoreLibrary &&o)
{
    cast(impl).operator=(std::move(cast(o.impl)));
    return *this;
}

ClientCoreLibrary::~ClientCoreLibrary()
{
    cast(impl).~ClientCoreLibraryPosix();
}
} // namespace Wayland
